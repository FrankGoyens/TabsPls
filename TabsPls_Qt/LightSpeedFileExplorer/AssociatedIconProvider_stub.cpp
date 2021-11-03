#include "AssociatedIconProvider.hpp"

#include "ExplicitStub.hpp"

bool AssociatedIconProvider::ComponentIsAvailable() { return false; }

AssociatedIconProvider& AssociatedIconProvider::Get() { throw ExplicitStubException{}; }

void AssociatedIconProvider::InitThread() {}

std::optional<QIcon> AssociatedIconProvider::FromPath(const FileSystem::RawPath& path) const {
    throw ExplicitStubException{};
}