#![allow(unused_imports, unused_parens,unused_variables,non_upper_case_globals)]

extern crate piston_window;

use piston_window::*;

use std::num;

const width: u32 = 1024;
const height: u32 = 768;

const step_size: u32 = 8;
const steps_x: u32 = width  / step_size;
const steps_y: u32 = height / step_size;

#[derive(Clone)]
enum WorldAtom {
    Empty,
    Solid([f64; 4]),
}

struct GameState {
    world: Vec<Vec<WorldAtom>>,
}

impl GameState {
    fn new(xy: &[usize;2] ) -> GameState {
        let mut state = GameState {
            world: vec![ vec![WorldAtom::Empty; xy[1]]; xy[0] ]
        };

        let (w, h) = state.world_size();
        let mut max_h = 10;
        let platform_width = 8;

        for iw in 0..(w/platform_width) {
            let modval = (iw%max_h) as usize;

            let mut val: usize = 0;

            if( 0 == (iw/max_h) || 0 == ((iw/max_h)%2)  ) {
                val = modval;
            }
            else {
                val = max_h-modval;
            }

            for iPlatWidth in 0..platform_width {
                state.world[(iw*platform_width+iPlatWidth) as usize][val as usize] = WorldAtom::Solid([0.5, 0.5, 0.5, 0.5]);
            }
        }

        state 
    }

    fn world_size(&self) -> (usize, usize) {
        (self.world.len(), self.world[0].len())
    }
}

fn main() {
    let mut window: PistonWindow =
        WindowSettings::new("Hello Piston!", [width, height])
        .exit_on_esc(true).build().unwrap();
    let mut x_view_pos = steps_x / 2;
    let mut flip = 0.0;

    //initialize world
    let game_state = GameState::new( &[(32*steps_x) as usize, steps_y as usize] );

    while let Some(e) = window.next() {  
        if let Some(Button::Mouse(button)) = e.press_args() {

            if( MouseButton::Left == button ) {
                 x_view_pos += 1;
            }
            if( MouseButton::Right == button ) {
                 x_view_pos -= 1;
            }
        }
        if let Some(Button::Keyboard(key)) = e.press_args() {
            println!("Pressed keyboard key '{:?}'", key);
             x_view_pos += 1;
        };

        window.draw_2d(&e, |context, graphics| {
            clear([1.0; 4], graphics);

            for x in 0..steps_x {
                for y in 0..steps_y {
                    

                    if let WorldAtom::Solid(color) = game_state.world[(x+x_view_pos) as usize][y as usize] {
                         rectangle([1.0, flip, 0.0, 1.0], // red
                            [(x*step_size) as f64, (height-y*step_size) as f64, step_size as f64, step_size as f64],
                            context.transform,
                            graphics);
                    }
                }
            }
        });
    }
}