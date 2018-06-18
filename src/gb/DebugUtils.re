let debug_print = a => {
  Js.log(a);
  a;
};

let toHex: int => string = [%raw
  int => "{
const num = int.toString(16);
let zeroes = num.length === 1 ? '000' : (num.length === 2 ? '00' : (num.length===3 ? '0' : ''));
return `0x${zeroes}${int.toString(16)}`;
}"
];
