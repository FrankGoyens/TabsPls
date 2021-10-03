#include "AssociatedIconProvider.hpp"

#include "ExplicitStub.hpp"

bool AssociatedIconProvider::ComponentIsAvailable() { return false; }

AssociatedIconProvider& AssociatedIconProvider::Get() { throw ExplicitStubException{}; }

std::optional<QIcon> AssociatedIconProvider::FromPath(const FileSystem::RawPath& path) const {
    throw ExplicitStubException{};
}