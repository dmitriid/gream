[%%debugger.chrome];

type command_return = {
  cpu_state: GameBoy.cpu_state,
  memory: GameBoy.memory,
};

type command = {
  cycles: int,
  no_of_args: int,
  label: string,
  command: (GameBoy.cpu_state, GameBoy.memory, list(int)) => command_return,
};

exception OPCODE_NOT_FOUND(string);
exception OPCODE_EXTENDED_NOT_FOUND(string);

let msb = (value: int) => value lsr 8;
let lsb = (value: int) => value land 0xff;
let to_word = (msb: int, lsb: int) => msb lsl 8 lor lsb;

let set_stack_pointer = (cpu_state: GameBoy.cpu_state, value) => {...cpu_state, stack_pointer: value};
let decrement_stack_pointer = (cpu_state: GameBoy.cpu_state) => {
  let sp = cpu_state.stack_pointer;
  {...cpu_state, stack_pointer: (sp - 1) mod 0xffff};
};
let increment_stack_pointer = (cpu_state: GameBoy.cpu_state) => {
  let sp = cpu_state.stack_pointer;
  {...cpu_state, stack_pointer: (sp + 1) mod 0xffff};
};

let push = (cpu_state: GameBoy.cpu_state, memory: GameBoy.memory, value) => {
  let dec1 = decrement_stack_pointer(cpu_state);
  let mem1 = Memory.write_byte(memory, dec1.stack_pointer, msb(value));
  let dec2 = decrement_stack_pointer(dec1);
  let mem2 = Memory.write_byte(mem1, dec2.stack_pointer, lsb(value));
  {cpu_state: dec2, memory: mem2};
};

let call = (cpu_state, memory, address) => {
  let result = push(cpu_state, memory, address);
  {
    cpu_state: {
      ...result.cpu_state,
      program_counter: address,
    },
    memory: result.memory,
  };
};

type xor_return = {
  flags: GameBoy.flags,
  result: int,
};
let xor = (_flags: GameBoy.flags, byte1: int, byte2: int) => {
  let result = byte1 lxor byte2;
  {
    flags: {
      zero: result == 0,
      operation: false,
      half_carry: false,
      carry: false,
    },
    result,
  };
};

type register =
  | A
  | B
  | C
  | D
  | E
  | H
  | L
  | DE
  | HL;

let get_register = (registers: GameBoy.registers, register) =>
  switch (register) {
  | A => registers.a
  | B => registers.b
  | C => registers.c
  | D => registers.d
  | E => registers.e
  | H => registers.h
  | L => registers.l
  | DE => to_word(registers.d, registers.e)
  | HL => to_word(registers.h, registers.l)
  };

let set_register = (registers: GameBoy.registers, register, value: int) =>
  switch (register) {
  | A => {...registers, a: value}
  | B => {...registers, b: value}
  | C => {...registers, c: value}
  | D => {...registers, d: value}
  | E => {...registers, e: value}
  | H => {...registers, h: value}
  | L => {...registers, l: value}
  | DE => {...registers, d: msb(value), e: lsb(value)}
  | HL => {...registers, h: msb(value), l: lsb(value)}
  };

let decrementHL = registers => {
  let oldHL = get_register(registers, HL);
  set_register(registers, HL, (oldHL - 1) mod 0xffff);
};

let incrementHL = registers => {
  let oldHL = get_register(registers, HL);
  set_register(registers, HL, (oldHL + 1) mod 0xffff);
};

let sub_bytes = (byte1, byte2) => {
  flags: {
    zero: byte1 - byte2 land 0xff == 0,
    operation: true,
    half_carry: 0x0f land byte2 > 0x0f land byte1,
    carry: byte2 > byte1,
  },
  result: (byte1 - byte2) mod 0xff,
};

let bit: (GameBoy.flags, int, int) => GameBoy.flags =
  (flags, byte_value, bit) =>
    if (bit < 8) {
      {...flags, operation: false, half_carry: true, zero: byte_value land 1 lsl bit != 0};
    } else {
      {...flags, operation: false, half_carry: true};
    };

let set_bit = (byte_value, position) => byte_value lor 1 lsl position land 0xff;

let decode = value =>
  switch (value) {
  | 0x00 => {cycles: 4, no_of_args: 0, label: "NOP", command: (cpu_state, memory, _value) => {cpu_state, memory}}
  | 0x06 => {
      cycles: 8,
      no_of_args: 1,
      label: "LD B, n",
      command: (cpu_state, memory, value) => {
        cpu_state: {
          ...cpu_state,
          registers: set_register(cpu_state.registers, B, List.nth(value, 0)),
        },
        memory,
      },
    }
  | 0x0e => {
      cycles: 8,
      no_of_args: 1,
      label: "LD C,d8",
      command: (cpu_state, memory, value) => {
        cpu_state: {
          ...cpu_state,
          registers: set_register(cpu_state.registers, C, List.nth(value, 0)),
        },
        memory,
      },
    }
  | 0x11 => {
      cycles: 12,
      no_of_args: 2,
      label: "LD DE,nn",
      command: (cpu_state, memory, value) => {
        cpu_state: {
          ...cpu_state,
          registers: set_register(cpu_state.registers, DE, to_word(List.nth(value, 1), List.nth(value, 0))),
        },
        memory,
      },
    }
  | 0x1A => {
      cycles: 8,
      no_of_args: 0,
      label: "LD A, (DE)",
      command: (cpu_state, memory, _value) => {
        cpu_state: {
          ...cpu_state,
          registers:
            set_register(cpu_state.registers, A, Memory.read_byte(memory, get_register(cpu_state.registers, DE))),
        },
        memory,
      },
    }
  | 0x20 => {
      cycles: 12 /* or 8 if no action taken*/,
      no_of_args: 1,
      label: "JR NZ,r8",
      command: (cpu_state, memory, value) =>
        if (! cpu_state.flags.zero) {
          {
            cpu_state: {
              ...cpu_state,
              program_counter: List.nth(value, 0),
            },
            memory,
          };
        } else {
          {cpu_state, memory};
        },
    }
  | 0x21 => {
      cycles: 12,
      no_of_args: 2,
      label: "LD HL, d16",
      command: (cpu_state, memory, value) => {
        cpu_state: {
          ...cpu_state,
          registers: set_register(cpu_state.registers, HL, to_word(List.nth(value, 1), List.nth(value, 0))),
        },
        memory,
      },
    }
  | 0x31 => {
      cycles: 12,
      no_of_args: 2,
      label: "LD SP, d16",
      command: (cpu_state, memory, value) => {
        let arg_value = to_word(List.nth(value, 1), List.nth(value, 0));
        let next_state = set_stack_pointer(cpu_state, arg_value);
        {cpu_state: next_state, memory};
      },
    }
  | 0x32 => {
      cycles: 8,
      no_of_args: 0,
      label: "LD (HLD), A",
      command: (cpu_state, memory, _value) => {
        let new_registers = decrementHL(cpu_state.registers);
        let a = get_register(cpu_state.registers, A);
        {
          cpu_state: {
            ...cpu_state,
            registers: new_registers,
          },
          memory: Memory.write_byte(memory, get_register(new_registers, HL), a),
        };
      },
    }
  | 0x3E => {
      cycles: 8,
      no_of_args: 1,
      label: "LD A, d8",
      command: (cpu_state, memory, value) => {
        cpu_state: {
          ...cpu_state,
          registers: set_register(cpu_state.registers, A, List.nth(value, 0)),
        },
        memory,
      },
    }
  | 0x77 => {
      cycles: 8,
      no_of_args: 0,
      label: "LD (HL), A",
      command: (cpu_state, memory, _value) => {
        cpu_state,
        memory:
          Memory.write_byte(memory, get_register(cpu_state.registers, HL), get_register(cpu_state.registers, A)),
      },
    }
  /* XOR */
  | 0xAF => {
      cycles: 4,
      no_of_args: 0,
      label: "XOR A, A",
      command: (cpu_state, memory, _value) => {
        let a = get_register(cpu_state.registers, A);
        let {result} = xor(cpu_state.flags, a, a);
        let registers = set_register(cpu_state.registers, A, result);
        {
          cpu_state: {
            ...cpu_state,
            registers,
          },
          memory,
        };
      },
    }
  | 0xCD => {
      cycles: 12,
      no_of_args: 2,
      label: "CALL nn",
      command: (cpu_state, memory, value) => {
        let result = call(cpu_state, memory, to_word(List.nth(value, 1), List.nth(value, 0)));
        {cpu_state: result.cpu_state, memory: result.memory};
      },
    }
  | 0xE0 => {
      cycles: 8,
      no_of_args: 1,
      label: "LD (n),A",
      command: (cpu_state, memory, value) => {
        cpu_state,
        memory: Memory.write_byte(memory, 0xff00 + List.nth(value, 0), get_register(cpu_state.registers, A)),
      },
    }
  | 0xE2 => {
      cycles: 8,
      no_of_args: 1,
      label: "LD (C),A",
      command: (cpu_state, memory, _value) => {
        cpu_state,
        memory:
          Memory.write_byte(
            memory,
            0xff00 + get_register(cpu_state.registers, C),
            get_register(cpu_state.registers, A),
          ),
      },
    }
  /* CP */
  | 0xFE => {
      cycles: 8,
      no_of_args: 1,
      label: "CP A, #",
      command: (cpu_state, memory, value) => {
        let a = get_register(cpu_state.registers, A);
        let {flags} = sub_bytes(a, List.nth(value, 0));
        {
          cpu_state: {
            ...cpu_state,
            flags,
          },
          memory,
        };
      },
    }
  | _ => raise(OPCODE_NOT_FOUND(DebugUtils.toHex(value)))
  };

let decode_extended = value =>
  switch (value) {
  /* BIT */
  | 0x7c => {
      cycles: 8,
      no_of_args: 0,
      label: "BIT 7, H",
      command: (cpu_state, memory, _value) => {
        cpu_state: {
          ...cpu_state,
          flags: bit(cpu_state.flags, get_register(cpu_state.registers, H), 7),
        },
        memory,
      },
    }

  | _ => raise(OPCODE_EXTENDED_NOT_FOUND(DebugUtils.toHex(value)))
  };
