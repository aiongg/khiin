#pragma once

#include "pch.h"

namespace Khiin {

//template <typename T, typename U, typename... Args>
//winrt::com_ptr<U> getComPtr(HRESULT (T::*accessor)(Args... args, U **ppObj), Args &&...args) {
//    auto ptr = winrt::com_ptr<ITfType>();
//    auto hr = &accessor(std::forward<Args>(args)..., ptr.put());
//    CHECK_RETURN_HRESULT(hr);
//    return ptr;
//}

// T = ITfContext, U = ITfContextView
// ITfContext::getTheThing
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
