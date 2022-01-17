#pragma once

#include "pch.h"

namespace Khiin {

// template <typename T, typename U, typename... Args>
// winrt::com_ptr<U> getComPtr(HRESULT (T::*accessor)(Args... args, U **ppObj), Args &&...args) {
//    auto ptr = winrt::com_ptr<ITfType>();
//    auto hr = &accessor(std::forward<Args>(args)..., ptr.put());
//    CHECK_RETURN_HRESULT(hr);
//    return ptr;
//}

template <typename D, typename T>
D *cast_as(T &&o) {
    return static_cast<D *>(o);
    // result.copy_from();
}

template <typename D, typename T>
D *get_self(T &&o) {
    return static_cast<D *>(o.get());
    // result.copy_from();
}

template <typename D, typename T>
winrt::com_ptr<D> as_self(T &&o) {
    auto result = winrt::com_ptr<D>();
    result.copy_from(static_cast<D *>(o.get()));
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

} // namespace Khiin
