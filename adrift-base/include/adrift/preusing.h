// Copyright (C) 2026 pointer-to-bios <pointer-to-bios@outlook.com>
// SPDX-License-Identifier: MIT

#pragma once

#include "adrift/algebra/matrix.h"
#include "adrift/constants.h"       // IWYU pragma: keep
#include "adrift/types/concepts.h"  // IWYU pragma: keep
#include "adrift/types/int.h"       // IWYU pragma: keep
#include "adrift/types/types.h"     // IWYU pragma: keep
#include "adrift/types/uchar.h"     // IWYU pragma: keep
#include "adrift/types/ustr.h"      // IWYU pragma: keep
#include "adrift/util/type_id.h"    // IWYU pragma: keep
#include "adrift/util/unwrap.h"     // IWYU pragma: keep

namespace adrift {

using type_id = util::type_id;

using vector2f = algebra::vector2f;
using vector3f = algebra::vector3f;
using matrix3f = algebra::matrix3f;
using matrix4f = algebra::matrix4f;

};  // namespace adrift
