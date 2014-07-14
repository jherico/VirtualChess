#include "Common.h"

#include <deque>
#include <iostream>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>

#pragma warning( disable : 4068 4244 4099 4305 4101)

#include <oglplus/images/load.hpp>
#include <oglplus/bound/texture.hpp>
#include <oglplus/bound/framebuffer.hpp>
#include <oglplus/bound/renderbuffer.hpp>

#include <oglplus/opt/resources.hpp>
#include <oglplus/opt/list_init.hpp>
#pragma warning( default : 4068 4244 4099 4305 4101)

#include <StandardPosition.h>
#include <openctmpp.h>

using namespace oglplus;
using boost::asio::ip::tcp;
using namespace std;


class telnet_client
{
public:
  enum { max_read_length = 512 };

  telnet_client(boost::asio::io_service& io_service, tcp::resolver::iterator endpoint_iterator)
    : io_service_(io_service), socket_(io_service)
  {
    connect_start(endpoint_iterator);
  }

  void write(const char msg) // pass the write data to the do_write function via the io service in the other thread
  {
    io_service_.post(boost::bind(&telnet_client::do_write, this, msg));
  }

  void write(const string msg) // pass the write data to the do_write function via the io service in the other thread
  {
    io_service_.post(boost::bind(&telnet_client::do_write_string, this, msg));
  }

  void close() // call the do_close function via the io service in the other thread
  {
    io_service_.post(boost::bind(&telnet_client::do_close, this));
  }

private:

  void connect_start(tcp::resolver::iterator endpoint_iterator)
  { // asynchronously connect a socket to the specified remote endpoint and call connect_complete when it completes or fails
    tcp::endpoint endpoint = *endpoint_iterator;
    socket_.async_connect(endpoint,
      boost::bind(&telnet_client::connect_complete,
      this,
      boost::asio::placeholders::error,
      ++endpoint_iterator));
  }

  void connect_complete(const boost::system::error_code& error, tcp::resolver::iterator endpoint_iterator)
  { // the connection to the server has now completed or failed and returned an error
    if (!error) // success, so start waiting for read data
      read_start();
    else if (endpoint_iterator != tcp::resolver::iterator())
    { // failed, so wait for another connection event
      socket_.close();
      connect_start(endpoint_iterator);
    }
  }

  void read_start(void)
  { // Start an asynchronous read and call read_complete when it completes or fails
    socket_.async_read_some(boost::asio::buffer(read_msg_, max_read_length),
      boost::bind(&telnet_client::read_complete,
      this,
      boost::asio::placeholders::error,
      boost::asio::placeholders::bytes_transferred));
  }

  virtual void on_read(char * bytes, size_t size) = 0;

  void read_complete(const boost::system::error_code& error, size_t bytes_transferred)
  { // the asynchronous read operation has now completed or failed and returned an error
    if (!error)
    { // read completed, so process the data
      on_read(read_msg_, bytes_transferred);
      //cout << "\n";
      read_start(); // start waiting for another asynchronous read again
    }
    else
      do_close();
  }

  void do_write(const char msg)
  { // callback to handle write call from outside this class
    bool write_in_progress = !write_msgs_.empty(); // is there anything currently being written?
    write_msgs_.push_back(msg); // store in write buffer
    if (!write_in_progress) // if nothing is currently being written, then start
      write_start();
  }

  void do_write_string(const string msg)
  { // callback to handle write call from outside this class
    bool write_in_progress = !write_msgs_.empty(); // is there anything currently being written?
    write_msgs_.insert(write_msgs_.end(), msg.begin(), msg.end()); // store in write buffer
    if (!write_in_progress) // if nothing is currently being written, then start
      write_start();
  }

  void write_start(void)
  { // Start an asynchronous write and call write_complete when it completes or fails
    boost::asio::async_write(socket_,
      boost::asio::buffer(&write_msgs_.front(), 1),
      boost::bind(&telnet_client::write_complete,
      this,
      boost::asio::placeholders::error));
  }

  void write_complete(const boost::system::error_code& error)
  { // the asynchronous read operation has now completed or failed and returned an error
    if (!error)
    { // write completed, so send next write data
      write_msgs_.pop_front(); // remove the completed data
      if (!write_msgs_.empty()) // if there is anthing left to be written
        write_start(); // then start sending the next item in the buffer
    }
    else
      do_close();
  }

  void do_close()
  {
    socket_.close();
  }

private:
  boost::asio::io_service& io_service_; // the main IO service that runs this connection
  tcp::socket socket_; // the socket this instance is connected to
  char read_msg_[max_read_length]; // data read from the socket
  deque<char> write_msgs_; // buffered write data
};

class telnet_line_client : public telnet_client
{
  std::deque<char> buffer;

public:
  telnet_line_client(boost::asio::io_service& io_service, tcp::resolver::iterator endpoint_iterator)
    : telnet_client(io_service, endpoint_iterator) {
  }

  virtual void on_read(char * bytes, size_t size) {
    buffer.insert(buffer.end(), bytes, bytes + size);
    std::deque<char>::iterator pos = std::find(buffer.begin(), buffer.end(), '\n');
    while (pos != buffer.end())  {
      std::string line; 
      line.assign(buffer.begin(), pos);
      on_line(line);
      buffer.erase(buffer.begin(), pos);
      buffer.pop_front();
      if (!buffer.empty() && ('\r' && buffer.at(0))) {
        buffer.pop_front();
      }
      pos = std::find(buffer.begin(), buffer.end(), '\n');
    }
  }

  bool sentLogin = false;
  enum State  {
    CONNECT,
    WAIT_LOGIN,
    WAIT_PASSWORD,
    LOGGED_IN,
    INTERFACE_SETUP,
  };

  State state{ CONNECT };
  const char * const INTERFACE_MACRO = 
    "iset defprompt 1\n"
    "iset gameinfo 1\n"
    "iset ms 1\n"
    "iset allresults 1\n"
    "iset startpos 1\n"
    "iset pendinfo 1\n"
    "iset nowrap 1\n"
    "set interface VirtualChess\n"
    "set style 12\n"
    "set bell 0\n"
    "set ptime 0\n"
//    "iset block 1\n"
    "iset lock 1\n"
    "games\n"
    "observe 16\n";

  virtual void on_line(std::string & line) {
    OutputDebugStringA(line.c_str());
    OutputDebugStringA("\n");
    switch (state) {
    case CONNECT:
      write("ariha\nborklu\n");
      state = LOGGED_IN;
//      state = WAIT_LOGIN;
      break;
    case WAIT_LOGIN:
      if (string::npos != line.find("login:")) {
        state = LOGGED_IN;
      }
      break;
    case WAIT_PASSWORD:
      if (string::npos != line.find("password:")) {
        write("borklu\n");
        state = LOGGED_IN;
      }
      break;
    case LOGGED_IN:
      write(INTERFACE_MACRO);
      state = INTERFACE_SETUP;
    default:
      break;
    }
  }
};


void loadCtm(Mesh & mesh, const std::string & data) {
  CTMimporter importer;
  importer.LoadData(data);

  int vertexCount = importer.GetInteger(CTM_VERTEX_COUNT);
  mesh.positions.resize(vertexCount);
  const float * ctmData = importer.GetFloatArray(CTM_VERTICES);
  for (int i = 0; i < vertexCount; ++i) {
    glm::vec4 pos(glm::make_vec3(ctmData + (i * 3)), 1);
    pos = mesh.model.top() * pos;
    pos /= pos.w;
    mesh.positions[i] = vec4(glm::make_vec3(&pos.x), 1);
  }

  if (importer.GetInteger(CTM_UV_MAP_COUNT) > 0) {
    const float * ctmData = importer.GetFloatArray(CTM_UV_MAP_1);
    mesh.texCoords.resize(vertexCount);
    for (int i = 0; i < vertexCount; ++i) {
      mesh.texCoords[i] = glm::make_vec2(ctmData + (i * 2));
    }
  }

  bool hasNormals = importer.GetInteger(CTM_HAS_NORMALS) ? true : false;
  if (hasNormals) {
    mesh.normals.resize(vertexCount);
    ctmData = importer.GetFloatArray(CTM_NORMALS);
    for (int i = 0; i < vertexCount; ++i) {
      mesh.normals[i] = vec4(glm::make_vec3(ctmData + (i * 3)), 1);
    }
  }

  int indexCount = 3 * importer.GetInteger(CTM_TRIANGLE_COUNT);
  const CTMuint * ctmIntData = importer.GetIntegerArray(CTM_INDICES);
  mesh.indices.resize(indexCount);
  for (int i = 0; i < indexCount; ++i) {
    mesh.indices[i] = *(ctmIntData + i);
  }
}

#define SET_PROJECTION(program) \
  Uniform<mat4>(program, Layout::Uniform::Projection).Set(Stacks::projection().top())

#define SET_MODELVIEW(program) \
  Uniform<mat4>(program, Layout::Uniform::ModelView).Set(Stacks::modelview().top())

namespace Piece {
  enum {
    PAWN,
    ROOK,
    KNIGHT,
    BISHOP,
    QUEEN,
    KING,
    COUNT
  };
}

Resource PIECE_RESOURCES[] = {
  Resource::MESHES_CHESS_PAWN_CTM,
  Resource::MESHES_CHESS_ROOK_CTM,
  Resource::MESHES_CHESS_KNIGHT_CTM,
  Resource::MESHES_CHESS_BISHOP_CTM,
  Resource::MESHES_CHESS_QUEEN_CTM,
  Resource::MESHES_CHESS_KING_CTM,
};
template <typename Function>
void for_each_square(Function f) {
  for (int row = 0; row < 8; ++row) {
    for (int col = 0; col < 8; ++col) {
      f(row, col);
    }
  }
}

class VirtualChess : public RiftApp {
  bool quit{ false };
  mat4 player;

  Program prog;
  Geometry pieces[6];
  AGChess::StandardPosition position;
  

  Geometry & getPieceGeometry(const AGChess::Piece & piece) {
    switch (piece.enumValue()) {
    case AGChess::Piece::KingPiece:
      return pieces[Piece::KING];
    case AGChess::Piece::QueenPiece:
      return pieces[Piece::QUEEN];
    case AGChess::Piece::KnightPiece:
      return pieces[Piece::KNIGHT];
    case AGChess::Piece::BishopPiece:
      return pieces[Piece::BISHOP];
    case AGChess::Piece::RookPiece:
      return pieces[Piece::ROOK];
    case AGChess::Piece::PawnPiece:
      return pieces[Piece::PAWN];
    }
    throw std::runtime_error("Uknown piece");
  }

public:

  VirtualChess(const RiftWrapperArgs & args) : RiftApp(args) {
    player = glm::inverse(glm::lookAt(vec3(0, 0, 0.5), vec3(0, 0, 0), vec3(0, 1, 0)));
    prog = GlUtils::getProgram(Resource::SHADERS_LIT_VS, Resource::SHADERS_LITCOLORED_FS);
    for (int i = 0; i < Piece::COUNT; ++i) {
      Mesh pieceMesh;
      pieceMesh.model.scale(1.6f);
      loadCtm(pieceMesh, Platform::getResourceString(PIECE_RESOURCES[i]));
      pieces[i].loadMesh(pieceMesh);
    }


    try
    {
      boost::asio::io_service io_service;
      // resolve the host name and port number to an iterator that can be used to connect to the server
      tcp::resolver resolver(io_service);
      tcp::resolver::query query("freechess.org", "5000");
      tcp::resolver::iterator iterator = resolver.resolve(query);
      // define an instance of the main class of this program
      telnet_line_client c(io_service, iterator);
      // run the IO service as a separate thread, so the main thread can block on standard input
      boost::thread t(boost::bind(&boost::asio::io_service::run, &io_service));
      //c.write('\r');
      //c.write('\n');
      while (1)
      {
//        char ch;
//        cin.get(ch); // blocking wait for standard input
//        if (ch == 3) // ctrl-C to end program
//          break;
//        c.write(ch);
        Platform::sleepMillis(100);
      }
      c.close(); // close the telnet client connection
      t.join(); // wait for the IO service thread to close
    }
    catch (exception& e)
    {
      cerr << "Exception: " << e.what() << "\n";
    }
  }

  virtual bool isDone() {
    return quit;
  }

  void updateState() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_KEYDOWN) {
        switch (event.key.keysym.sym) {
        case SDLK_r:
          ovrHmd_ResetSensor(hmd);
          return;

        case SDLK_ESCAPE:
          quit = true;
          return;
        }
      }

      if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) {
        quit = true;
        return;
      }
      CameraControl::instance().onEvent(event);
    }

    CameraControl::instance().applyInteraction(player);
    Stacks::modelview().top() = glm::inverse(player);
  }

  void renderBoard() {
    prog.Use();
    SET_PROJECTION(prog);
    SET_MODELVIEW(prog);
    Uniform<vec4>(prog, 7).Set(vec4(0.3, 0.3, 0.3, 1.0));
    Uniform<int>(prog, "LightCount").Set(1);
    Uniform<int>(prog, "LightCount").Set(1);
    Uniform<vec4>(prog, "LightPosition[0]").Set(vec4(1));
    Uniform<vec4>(prog, "LightColor[0]").Set(vec4(1));

    MatrixStack & mv = Stacks::modelview();
    mv.withPush([&]{
      mv.scale(GlUtils::CHESS_SCALE);
      mv.translate(vec3(-3.5, 0, -3.5));
      for_each_square([&](int row, int col){
        mv.withPush([&] {
          AGChess::ColoredPiece agPiece = position.at(AGChess::Square((7 - row) * 8 + col));
          if (!agPiece.isValid()) {
            return;
          }
          mv.translate(vec3(col, 0, row));

          vec3 color = Colors::white;
          if (agPiece.color() != AGChess::Color::WhiteColor) {
            color = Colors::darkGray;
          } else {
            mv.rotate(PI, GlUtils::Y_AXIS);

          }
          Uniform<vec4>(prog, "Color").Set(vec4(color, 1));
          SET_MODELVIEW(prog);
          Geometry & pieceGeometry = getPieceGeometry(agPiece.piece());
          pieceGeometry.bind();
          pieceGeometry.draw();
        });
      });
    });
    NoProgram().Use();
    NoVertexArray().Bind();

    Render::renderGeometry(
      GlUtils::getProgram(Resource::SHADERS_COLORED_VS, Resource::SHADERS_COLORED_FS),
      GlUtils::getChessBoardGeometry());
  }

  void drawScene() {
    gl.Clear().DepthBuffer();
    Render::renderSkybox(Resource::IMAGES_SKY_CITY_XNEG_PNG);
    renderBoard();
  }
};

RUN_OVR_APP(RiftWrapperApp<VirtualChess>);



