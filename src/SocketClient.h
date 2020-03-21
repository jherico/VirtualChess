#pragma once

class SocketClient {
public:
  //typedef boost::circular_buffer<char> WriteBuffer;
  typedef std::deque<char> ReadBuffer;
  //typedef boost::asio::ip::tcp tcp;
private:
  enum {
    max_read_length = 512
  };

  //boost::asio::io_service io_service;
  //boost::asio::io_service::work work;
  //boost::thread serviceThread;
  //tcp::socket  socket;

  char socketReadBuffer[max_read_length];

  // TODO block on full buffer
  //WriteBuffer writeBuffer{8192};
  //ReadBuffer readBuffer;

public:

  //SocketClient() :
  //  work(io_service), serviceThread(boost::bind(&boost::asio::io_service::run, &io_service)), socket(io_service) {
  //}

  void write(const char msg) {
    //io_service.post(boost::bind(&SocketClient::doWrite, this, msg));
  }

  void write(const std::string msg) {
    //io_service.post(boost::bind(&SocketClient::doWriteString, this, msg));
  }

  void close() // call the do_close function via the io service in the other thread
  {
    //io_service.post(boost::bind(&SocketClient::doClose, this));
  }

  //void connect(tcp::resolver::iterator & endpoint_iterator) {
  //  connectStart(endpoint_iterator);
  //}

  virtual void onRead(ReadBuffer & readBuffer) = 0;

private:

  //void connectStart(boost::asio::ip::tcp::resolver::iterator & endpointIterator) {
  //  tcp::endpoint endpoint = *endpointIterator;
  //  socket.async_connect(endpoint,
  //      boost::bind(&SocketClient::connectComplete, this,
  //          boost::asio::placeholders::error, ++endpointIterator));
  //}

  //void connectComplete(const boost::system::error_code& error,
  //    tcp::resolver::iterator endpoint_iterator) {
  //  if (error) {
  //    std::string errorString = error.message();
  //    if (endpoint_iterator != tcp::resolver::iterator()) {
  //      socket.close();
  //      connectStart(endpoint_iterator);
  //    }
  //    return;
  //  }
  //  readStart();
  //}

  //void readStart(void) { // Start an asynchronous read and call read_complete when it completes or fails
  //  socket.async_read_some(
  //      boost::asio::buffer(socketReadBuffer, max_read_length),
  //      boost::bind(&SocketClient::readComplete, this,
  //          boost::asio::placeholders::error,
  //          boost::asio::placeholders::bytes_transferred));
  //}

  //void readComplete(const boost::system::error_code& error, size_t bytes_transferred) {
  //  if (error) {
  //    doClose();
  //    return;
  //  }
  //  readBuffer.insert(readBuffer.end(), socketReadBuffer,
  //      socketReadBuffer + bytes_transferred);

  //  onRead(readBuffer);
  //  readStart(); // start waiting for another asynchronous read again
  //}

  //void doWrite(const char msg) { // callback to handle write call from outside this class
  //  bool write_in_progress = !writeBuffer.empty(); // is there anything currently being written?
  //  writeBuffer.push_back(msg); // store in write buffer
  //  if (!write_in_progress) // if nothing is currently being written, then start
  //    writeStart();
  //}

  //void doWriteString(const std::string msg) {
  //  bool write_in_progress = !writeBuffer.empty();
  //  writeBuffer.insert(writeBuffer.end(), msg.begin(), msg.end());
  //  if (!write_in_progress) {
  //    writeStart();
  //  }
  //}

  //// TODO write more than one byte at a time
  //void writeStart(void) {
  //  boost::circular_buffer<char>::array_range ar = writeBuffer.array_one();
  //  boost::asio::async_write(socket,
  //      boost::asio::buffer(ar.first, ar.second),
  //      boost::bind(&SocketClient::writeComplete, this,
  //          boost::asio::placeholders::error,
  //          boost::asio::placeholders::bytes_transferred));
  //}

  //void writeComplete(const boost::system::error_code& error,
  //    size_t bytes_transferred) {
  //  if (error) {
  //    doClose();
  //    return;
  //  }
  //  writeBuffer.erase(writeBuffer.begin(),
  //      writeBuffer.begin() + bytes_transferred);
  //  if (!writeBuffer.empty()) {
  //    writeStart();
  //  }
  //}

  //void doClose() {
  //  socket.close();
  //}
};
