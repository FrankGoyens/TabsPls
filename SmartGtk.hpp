#pragma once

#include <memory>
#include <type_traits>

namespace SmartGtk
{
    template<typename NewFunction, typename UnrefFunction, typename... Args>
    auto MakeObject(NewFunction newFunction, UnrefFunction unrefFunction, Args&&... args)
    {
        using ObjectPtrT = typename std::invoke_result<NewFunction, Args...>::type;
        using ObjectT = typename std::remove_pointer<ObjectPtrT>::type;
        return std::unique_ptr<ObjectT, UnrefFunction>(newFunction(std::forward<Args>(args)...), unrefFunction);
    }
}

