#ifndef CRESULT_H_INCLUDED
#define CRESULT_H_INCLUDED
#include "ccommon.h"
#include "forward.h"
#include <stdlib.h>
#include <assert.h>

#endif // CRESULT_H_INCLUDED

#ifndef i_val
#define i_val int
#endif

#ifndef _i_prefix
#define _i_prefix cresult_
#endif
#include "template.h"

#if !c_option(c_is_fwd)
_cx_deftypes(_c_cresult_types, _cx_self, i_val);
#endif

STC_INLINE _cx_self
_cx_memb(_ok)(i_val value) {
    return c_make(_cx_self){.error_code = 0, .empty = false, .value = value};
}

STC_INLINE i_val
_cx_memb(_unwrap)(_cx_self self) {
    assert(self.error_code == 0);
    return self.value;
}

STC_INLINE _cx_self
_cx_memb(_err)(uint8_t error_code) {
    return c_make(_cx_self){.error_code = error_code, .empty = false};
}

STC_INLINE _cx_self
_cx_memb(_empty)() {
    return c_make(_cx_self){.error_code = 0, .empty = true};
}

STC_INLINE bool
_cx_memb(_is_err)(_cx_self self) {
    return self.error_code != 0;
}

STC_INLINE bool
_cx_memb(_is_empty)(_cx_self self) {
    return self.empty == true;
}

// #if !c_option(c_is_fwd)
// _cx_deftypes(_c_cresult_types, _cx_self, i_val);
// #endif

// STC_INLINE _cx_self
// _cx_memb(_with)(i_val value) {
//     return c_make(_cx_self){.error_code = 0, .empty = false, .value = value};
// }

// STC_INLINE _cx_self
// _cx_memb(_err)(uint8_t error_code) {
//     return c_make(_cx_self){.error_code = error_code, .empty = false};
// }

// STC_INLINE _cx_self
// _cx_memb(_empty)() {
//     return c_make(_cx_self){.error_code = 0, .empty = true};
// }

// STC_INLINE bool
// _cx_memb(_is_ok)(_cx_self self) {
//     return self.error_code == 0;
// }

// STC_INLINE bool
// _cx_memb(_is_empty)(_cx_self self) {
//     return self.empty == true;
// }

// STC_INLINE i_val
// _cx_memb(_unwrap)(_cx_self self) {
//     assert(self.error_code == 0);
//     return self.value;
// }

#include "template.h"
