// RUN: clang-cc -fsyntax-only -verify %s

void abcdefghi12(void) {
 const char (*ss)[12] = &__func__;
 static int arr[sizeof(__func__)==12 ? 1 : -1];
}

char *X = __func__; // expected-warning {{predefined identifier is only valid}} \
                       expected-warning {{initializing 'char const [1]' discards qualifiers, expected 'char *'}}

void a() {
  __func__[0] = 'a';  // expected-error {{variable is not assignable}}
}

// rdar://6097892 - GCC permits this insanity.
const char *b = __func__;  // expected-warning {{predefined identifier is only valid}}
const char *c = __FUNCTION__; // expected-warning {{predefined identifier is only valid}}
const char *d = __PRETTY_FUNCTION__; // expected-warning {{predefined identifier is only valid}}

