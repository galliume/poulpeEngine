module Poulpe.Core.Network;

void Server::bind(std::string const& port)
{
  _Pimpl->bind(port);
}

void Server::close()
{
  _Pimpl->close();
}

void Server::listen()
{
  _Pimpl->listen();
}

void Server::read()
{
  _Pimpl->read();
}

void Server::send(std::string message)
{
  _Pimpl->send(message);
}
