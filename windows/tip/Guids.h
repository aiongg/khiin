#pragma once

#include "pch.h"

namespace khiin::win32::guids {

inline constexpr int kGuidSize = 39;

static inline std::wstring String(GUID guid) {
    auto ret = std::wstring(kGuidSize, L'?');
    auto size = ::StringFromGUID2(guid, &ret[0], kGuidSize);
    if (size > 0) {
        return ret;
    }
    return std::wstring();
}

// From <InputScopes.h>
// 1713dd5a-68e7-4a5b-9af6-592a595c778d
inline constexpr GUID kPropInputScope = {0x1713dd5a, 0x68e7, 0x4a5b, {0x9a, 0xf6, 0x59, 0x2a, 0x59, 0x5c, 0x77, 0x8d}};

// 829893f6-728d-11ec-8c6e-e0d46491b35a
inline constexpr GUID kTextService = {0x829893f6, 0x728d, 0x11ec, {0x8c, 0x6e, 0xe0, 0xd4, 0x64, 0x91, 0xb3, 0x5a}};

// 829893f7-728d-11ec-8c6e-e0d46491b35a
inline constexpr GUID kLanguageProfile = {0x829893f7, 0x728d, 0x11ec, {0x8c, 0x6e, 0xe0, 0xd4, 0x64, 0x91, 0xb3, 0x5a}};

// 829893f8-728d-11ec-8c6e-e0d46491b35a
inline constexpr GUID kDisplayAttrInput = {
    0x829893f8, 0x728d, 0x11ec, {0x8c, 0x6e, 0xe0, 0xd4, 0x64, 0x91, 0xb3, 0x5a}};

// 829893f9-728d-11ec-8c6e-e0d46491b35a
inline constexpr GUID kDisplayAttrConverted = {
    0x829893f9, 0x728d, 0x11ec, {0x8c, 0x6e, 0xe0, 0xd4, 0x64, 0x91, 0xb3, 0x5a}};

// 829893fa-728d-11ec-8c6e-e0d46491b35a
inline constexpr GUID kCandidateWindow = {0x829893fa, 0x728d, 0x11ec, {0x8c, 0x6e, 0xe0, 0xd4, 0x64, 0x91, 0xb3, 0x5a}};

// 829893fb-728d-11ec-8c6e-e0d46491b35a
inline constexpr GUID kDisplayAttrFocused = {
    0x829893fb, 0x728d, 0x11ec, {0x8c, 0x6e, 0xe0, 0xd4, 0x64, 0x91, 0xb3, 0x5a}};

// 829893fc-728d-11ec-8c6e-e0d46491b35a
inline constexpr GUID kConfigChangedCompartment = {
    0x829893fc, 0x728d, 0x11ec, {0x8c, 0x6e, 0xe0, 0xd4, 0x64, 0x91, 0xb3, 0x5a}};

// 829893fd-728d-11ec-8c6e-e0d46491b35a
inline constexpr GUID kPreservedKeyOnOff = {
    0x829893fd, 0x728d, 0x11ec, {0x8c, 0x6e, 0xe0, 0xd4, 0x64, 0x91, 0xb3, 0x5a}};

// 829893fe-728d-11ec-8c6e-e0d46491b35a
inline constexpr GUID kPreservedKeySwitchMode = {
    0x829893fe, 0x728d, 0x11ec, {0x8c, 0x6e, 0xe0, 0xd4, 0x64, 0x91, 0xb3, 0x5a}};

// 829893ff-728d-11ec-8c6e-e0d46491b35a
inline constexpr GUID kPreservedKeyFullWidthSpace = {
    0x829893ff, 0x728d, 0x11ec, {0x8c, 0x6e, 0xe0, 0xd4, 0x64, 0x91, 0xb3, 0x5a}};

// 82989400-728d-11ec-8c6e-e0d46491b35a
inline constexpr GUID kResetUserdataCompartment = {
    0x82989400, 0x728d, 0x11ec, {0x8c, 0x6e, 0xe0, 0xd4, 0x64, 0x91, 0xb3, 0x5a}};

// 82989401-728d-11ec-8c6e-e0d46491b35a
// inline constexpr GUID kUnusedGuid = {0x82989401, 0x728d, 0x11ec, {0x8c, 0x6e, 0xe0, 0xd4, 0x64, 0x91, 0xb3, 0x5a}};

// 82989402-728d-11ec-8c6e-e0d46491b35a
// inline constexpr GUID kUnusedGuid = {0x82989402, 0x728d, 0x11ec, {0x8c, 0x6e, 0xe0, 0xd4, 0x64, 0x91, 0xb3, 0x5a}};

// 82989403-728d-11ec-8c6e-e0d46491b35a
// inline constexpr GUID kUnusedGuid = {0x82989403, 0x728d, 0x11ec, {0x8c, 0x6e, 0xe0, 0xd4, 0x64, 0x91, 0xb3, 0x5a}};

// 82989404-728d-11ec-8c6e-e0d46491b35a
// inline constexpr GUID kUnusedGuid = {0x82989404, 0x728d, 0x11ec, {0x8c, 0x6e, 0xe0, 0xd4, 0x64, 0x91, 0xb3, 0x5a}};

// 82989405-728d-11ec-8c6e-e0d46491b35a
// inline constexpr GUID kUnusedGuid = {0x82989405, 0x728d, 0x11ec, {0x8c, 0x6e, 0xe0, 0xd4, 0x64, 0x91, 0xb3, 0x5a}};

// 82989406-728d-11ec-8c6e-e0d46491b35a
// inline constexpr GUID kUnusedGuid = {0x82989406, 0x728d, 0x11ec, {0x8c, 0x6e, 0xe0, 0xd4, 0x64, 0x91, 0xb3, 0x5a}};

// 82989407-728d-11ec-8c6e-e0d46491b35a
// inline constexpr GUID kUnusedGuid = {0x82989407, 0x728d, 0x11ec, {0x8c, 0x6e, 0xe0, 0xd4, 0x64, 0x91, 0xb3, 0x5a}};

// 82989408-728d-11ec-8c6e-e0d46491b35a
// inline constexpr GUID kUnusedGuid = {0x82989408, 0x728d, 0x11ec, {0x8c, 0x6e, 0xe0, 0xd4, 0x64, 0x91, 0xb3, 0x5a}};

// 82989409-728d-11ec-8c6e-e0d46491b35a
// inline constexpr GUID kUnusedGuid = {0x82989409, 0x728d, 0x11ec, {0x8c, 0x6e, 0xe0, 0xd4, 0x64, 0x91, 0xb3, 0x5a}};

// 8298940a-728d-11ec-8c6e-e0d46491b35a
// inline constexpr GUID kUnusedGuid = {0x8298940a, 0x728d, 0x11ec, {0x8c, 0x6e, 0xe0, 0xd4, 0x64, 0x91, 0xb3, 0x5a}};

// 8298940b-728d-11ec-8c6e-e0d46491b35a
// inline constexpr GUID kUnusedGuid = {0x8298940b, 0x728d, 0x11ec, {0x8c, 0x6e, 0xe0, 0xd4, 0x64, 0x91, 0xb3, 0x5a}};

// 8298940c-728d-11ec-8c6e-e0d46491b35a
// inline constexpr GUID kUnusedGuid = {0x8298940c, 0x728d, 0x11ec, {0x8c, 0x6e, 0xe0, 0xd4, 0x64, 0x91, 0xb3, 0x5a}};

// 8298940d-728d-11ec-8c6e-e0d46491b35a
// inline constexpr GUID kUnusedGuid = {0x8298940d, 0x728d, 0x11ec, {0x8c, 0x6e, 0xe0, 0xd4, 0x64, 0x91, 0xb3, 0x5a}};

// 8298940e-728d-11ec-8c6e-e0d46491b35a
// inline constexpr GUID kUnusedGuid = {0x8298940e, 0x728d, 0x11ec, {0x8c, 0x6e, 0xe0, 0xd4, 0x64, 0x91, 0xb3, 0x5a}};

// 8298940f-728d-11ec-8c6e-e0d46491b35a
// inline constexpr GUID kUnusedGuid = {0x8298940f, 0x728d, 0x11ec, {0x8c, 0x6e, 0xe0, 0xd4, 0x64, 0x91, 0xb3, 0x5a}};

} // namespace khiin::win32::guids