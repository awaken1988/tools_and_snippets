extern crate piston_window;

use piston_window::*;

use std::num;

const width: u32 = 1024;
const height: u32 = 768;

const step_size: u32 = 32;
const steps_x: u32 = width  / step_size;
const steps_y: u32 = height / step_size;

fn main() {
    let mut window: PistonWindow =
        WindowSettings::new("Hello Piston!", [width, height])
        .exit_on_esc(true).build().unwrap();
    let mut counter = 0;
    let mut flip = 0.0;

    //let mut events = Events::new(EventSettings::new().lazy(false));
    while let Some(e) = window.next() {  
        if let Some(Button::Mouse(button)) = e.press_args() {

            if( MouseButton::Left == button ) {
                 counter = counter + 1;
            }
            if( MouseButton::Right == button ) {
                 counter = if counter > 0 {counter - 1} else {counter} ;
            }
        }
        if let Some(Button::Keyboard(key)) = e.press_args() {
            println!("Pressed keyboard key '{:?}'", key);
        };

        window.draw_2d(&e, |context, graphics| {
            clear([1.0; 4], graphics);

            for x in 0..steps_x {
                for y in 0..steps_y {
                    if 0 == (x+y*counter)%(4) {
                        continue;
                    }

                    flip = flip + 0.1;
                    if flip > 0.3 {
                        flip = 0.0
                    }

                     rectangle([1.0, flip, 0.0, 1.0], // red
                      [(x*step_size) as f64, (y*step_size) as f64, step_size as f64, step_size as f64],
                      context.transform,
                      graphics);
                }
            }
        });
    }
}