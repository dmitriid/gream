[%%debugger.chrome];

type next_arg_return = {
  cpu_state: GameBoy.cpu_state,
  value: list(int),
};

let rec read_next_arg_1: (GameBoy.cpu_state, GameBoy.memory, int, list(int)) => next_arg_return =
  (cpu_state, memory, arg, value) =>
    switch (arg) {
    | 0 => {
        cpu_state: {
          ...cpu_state,
          program_counter: cpu_state.program_counter + 1,
        },
        value,
      }
    | _ =>
      let next = Memory.read_byte(memory, cpu_state.program_counter);
      let new_cpu_state = {...cpu_state, program_counter: cpu_state.program_counter + 1};
      read_next_arg_1(new_cpu_state, memory, arg - 1, value @ [next]);
    };

let read_next_arg = (cpu_state, memory, arg) => read_next_arg_1(cpu_state, memory, arg, []);

let init = () => {
  let initial_state: GameBoy.cpu_state = {
    program_counter: 0,
    stack_pointer: 0,
    flags: {
      operation: false,
      zero: false,
      half_carry: false,
      carry: false,
    },
    registers: {
      b: 0,
      a: 0,
      c: 0,
      d: 0,
      e: 0,
      h: 0,
      l: 0,
    },
    clock: 0,
    stop: false,
  };
  initial_state;
};

type fork_execution_result = {
  op: Opcodes.command,
  new_program_counter: int,
};

let loop = (cpu_state: GameBoy.cpu_state, memory) => {
  let program_counter = cpu_state.program_counter;
  let byte = Memory.read_byte(memory, program_counter);

  let {op, new_program_counter} =
    if (byte == 0xcb) {
      let extra = Memory.read_byte(memory, program_counter + 1);
      {op: Opcodes.decode_extended(extra), new_program_counter: program_counter + 1};
    } else {
      {op: Opcodes.decode(byte), new_program_counter: program_counter};
    };

  op.label |> DebugUtils.debug_print |> ignore;

  let result = read_next_arg({...cpu_state, program_counter: new_program_counter}, memory, op.no_of_args);

  op.command(result.cpu_state, memory, result.value);
};
