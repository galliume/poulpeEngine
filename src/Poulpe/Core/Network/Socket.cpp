module Poulpe.Core.Network;

Socket::Socket(Socket* socket)
{
  _Pimpl = std::unique_ptr<Socket>(socket);
}

Socket::~Socket()
{
  _Pimpl->close();
}

void Socket::close()
{
  _Pimpl->close();
}

void Socket::bind(std::string const& ip, unsigned short const port)
{
  _Pimpl->bind(ip, port);
}

void Socket::connect()
{
  _Pimpl->connect();
}

void Socket::listen()
{
  _Pimpl->listen();
}

void Socket::read()
{
  _Pimpl->read();
}
