#pragma once

#include "pch.h"

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable : 4251 6385 26495 26812)
#endif

#include "proto/messages.pb.h"

#ifdef _WIN32
#pragma warning(pop)
#endif

namespace khiin::win32 {

template <typename Derived, typename T>
Derived *cast_as(T &&o) {
    return static_cast<Derived *>(o);
    // result.copy_from();
}

template <typename Derived, typename T>
Derived *get_self(T &&o) {
    return static_cast<Derived *>(o.get());
    // result.copy_from();
}

template <typename Derived, typename T>
winrt::com_ptr<Derived> as_self(T &&o) {
    auto result = winrt::com_ptr<Derived>();
    result.copy_from(static_cast<Derived *>(o.get()));
    return result;
}

template <typename T, typename U>
winrt::com_ptr<U> getComPtr(T *source, HRESULT (T::*fn)(U **ppObj)) {
    auto ptr = winrt::com_ptr<U>();
    auto hr = (source->*fn)(ptr.put());
    CHECK_HRESULT(hr);
    return ptr;
}

template <typename T, typename U>
winrt::com_ptr<U> getComPtr(winrt::com_ptr<T> source, HRESULT (T::*fn)(U **ppObj)) {
    auto ptr = winrt::com_ptr<U>();
    auto hr = (source.get()->*fn)(ptr.put());
    CHECK_HRESULT(hr);
    return ptr;
}

template <typename T, typename U, typename... Args>
winrt::com_ptr<U> getComPtr(T *source, HRESULT (T::*fn)(Args... args, U **ppObj), Args... args) {
    auto ptr = winrt::com_ptr<U>();
    auto hr = (source->*fn)(args..., ptr.put());
    CHECK_HRESULT(hr);
    return ptr;
}

template <typename T, typename U, typename... Args>
winrt::com_ptr<U> getComPtr(winrt::com_ptr<T> source, HRESULT (T::*fn)(Args... args, U **ppObj), Args... args) {
    auto ptr = winrt::com_ptr<U>();
    auto hr = (source.get()->*fn)(args..., ptr.put());
    CHECK_HRESULT(hr);
    return ptr;
}

//+---------------------------------------------------------------------------
//
// handlers
//
//----------------------------------------------------------------------------

struct registry_traits {
    using type = HKEY;

    static void close(type value) noexcept {
        WINRT_VERIFY_(ERROR_SUCCESS, ::RegCloseKey(value));
    }

    static constexpr type invalid() noexcept {
        return nullptr;
    }
};

using registry_key = winrt::handle_type<registry_traits>;

} // namespace khiin::win32
