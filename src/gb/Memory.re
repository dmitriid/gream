[%%debugger.chrome];

let init = bootstrap => {
  let initial_memory: GameBoy.memory = {bios: bootstrap, rom: [], gpu: [], ext: [], working: Array.make(0x10000, 0)};
  initial_memory;
};

let read_byte = (memory: GameBoy.memory, address) =>
  switch (address land 0xF000) {
  | 0x0000 when address < 0x0100 => Js_typed_array.Uint8Array.unsafe_get(memory.bios, address)
  | _ => memory.working[address]
  };
let write_byte: (GameBoy.memory, int, int) => GameBoy.memory =
  (memory, address, value) => {
    memory.working[address] = value;
    memory;
  };
