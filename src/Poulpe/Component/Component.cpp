module Poulpe.Component.Component;

void Component::setOwner(IDType owner) { _owner = owner; }

void Component::operator()(double const delta_time, Mesh * mesh)
{
  std::visit([&](auto& component) {
    if constexpr (hasCallOperator<decltype(*component)>) {
      (*component)(delta_time, mesh);
    }
  }, _pimpl);
}
