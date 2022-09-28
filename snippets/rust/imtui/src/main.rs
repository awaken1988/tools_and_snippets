use std::io::{stdout, Write};
use crossterm::{cursor, terminal, execute, style, event, event::{Event, read, KeyCode}, ExecutableCommand, QueueableCommand};

mod imtui;

use imtui::{*};

fn main() -> () {
    let mut w_0_0 = imtui::Label::new("Hello"); 
    let mut w_0_1 = imtui::Label::new("World!");
    let mut w_1_0 = imtui::Label::new("Hello ");
    let term_size = terminal::size().unwrap();

    loop {
        let mut layout = imtui::TableLayout::new();
        imtui::setup_screen();

        layout.add(&mut w_0_0, Size2D{x: 0, y: 0});
        layout.add(&mut w_0_1, Size2D{x: 1, y: 0});
        layout.add(&mut w_1_0, Size2D{x: 0, y: 1});
        layout.draw(Size2D::zero(), Size2D{x: term_size.0 as usize, y: term_size.1 as usize});
        
        let e = read().unwrap();
        match e {
            Event::Key(e) if e.code == KeyCode::Esc => {
                return;
            }
            Event::Key(e) if e.code == KeyCode::F(5) => {
                
            }
            Event::Key(e) if e.code == KeyCode::Up => {
            
            }
            Event::Key(e) if e.code == KeyCode::Down => {
                
            }
            Event::Key(e) if e.code == KeyCode::Enter => {
               
            }
            _ => {
                continue
            }
        }

        
    }

    
}
