[%bs.raw {|require('./index.css')|}];

[@bs.module "./registerServiceWorker"] external register_service_worker : unit => unit = "default";

ReactDOMRe.renderToElementWithId(<App message="Welcome to React and Reason" />, "root");

ROM.load("roms/bootstrap.bin")
|> Js.Promise.then_(blob => Memory.init(blob) |> DebugUtils.debug_print |> Js.Promise.resolve)
|> Js.Promise.then_(memory => {
     let init_cpu_state = CPU.init() |> DebugUtils.debug_print;

     let is_stop = ref(init_cpu_state.stop);
     let x = ref(256);

     let cpu = ref(init_cpu_state);
     let mem = ref(memory);

     while (! is_stop^ && x^ != 0) {
       let new_state = CPU.loop(cpu^, mem^);
       is_stop := new_state.cpu_state.stop;
       x := x^ - 1;

       cpu := new_state.cpu_state;
       mem := new_state.memory;
     };

     Js.Promise.resolve();
   });

register_service_worker();
