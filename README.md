Inspired by [Why did I spend 1.5 months creating a Gameboy emulator?](http://blog.rekawek.eu/2017/02/09/coffee-gb/).

# What

This is a project that may or may not go anywhere:

- I'm still unsure about ReasonML
- I know nothing about writing emulators
- I may or may not have time or willpower to finish this

Started this as a way to learn ReasonML a bit.

In many ways it's just a copy-paste of code from the blog post above (namely, when it was 
in [this state](https://github.com/trekawek/coffee-gb/tree/e6230db0b34521b2d2b33b2dac0773f0476a32a2)).

## Code

Your entry point is `index.re`.

Even though ReasonML/OCaml have mutability, it's much easier to work with immutable values. However, 
this means carrying all state alround. If you're not familiar with this, you may struggle (after several
years with Erlang and *then* a few years with JS/TypeScript/PHP it took me a while to get back into
the immutable mindset again).

I'm currently only using mutable arrays in `Memory.re` because it was faster to update arrays this way :)

## References

- [Why did I spend 1.5 months creating a Gameboy emulator?](http://blog.rekawek.eu/2017/02/09/coffee-gb/)
- [Pan Docs](http://gbdev.gg8.se/wiki/articles/Pan_Docs)
- [Implementing GameBoy emulator in Javascript](http://imrannazar.com/GameBoy-Emulation-in-JavaScript:-The-CPU)
- [Gameboy Instruction Set](http://www.pastraiser.com/cpu/gameboy/gameboy_opcodes.html)
- Google :) 

# How

- Follow [Quickstart](https://reasonml.github.io/docs/en/quickstart-javascript.html) and [Editor Setup](https://reasonml.github.io/docs/en/global-installation.html) to install all the required tools:
  - At the time of this writing it was `npm install -g bs-platform` and `npm install -g reason-cli@3.2.0-darwin`
- `yarn start`, and open console.
- if you want to develop this, VS Code is probably the code editor of choice for ReasonML right now 
(IDEA's plugin is somewhat lacking, but workable)

I haven't thoroughly checked it, but it should go through the boot sequence until it starts checking 
for Nintendo logo. I think :)

# Other

This project was bootstrapped with [Create React App](https://github.com/facebookincubator/create-react-app). That's why it currently relies on React (it shouldn't) 

