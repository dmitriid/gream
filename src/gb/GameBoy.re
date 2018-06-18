type flags = {
  zero: bool, /* Z */
  operation: bool, /* N */
  half_carry: bool, /* H */
  carry: bool /* C */
};

type registers = {
  a: int,
  b: int,
  c: int,
  d: int,
  e: int,
  h: int,
  l: int,
};

type cpu_state = {
  program_counter: int,
  stack_pointer: int,
  flags,
  registers,
  clock: int,
  stop: bool,
};

type memory = {
  bios: Js_typed_array.Uint8Array.t,
  rom: list(int),
  gpu: list(int),
  ext: list(int),
  working: array(int),
};
