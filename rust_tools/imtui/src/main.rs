use std::io::{stdout, Write};
use crossterm::{cursor, terminal, execute, style, event, event::{Event, read, KeyCode}, ExecutableCommand, QueueableCommand};

mod imtui;

use imtui::{*};

fn main() -> () {
    let mut lbl_0_0 = Label::new(".x=0; y=0.......");
    let mut lbl_1_1 = Label::new("_x=1; y=1_");
    let mut lbl_2_2 = Label::new("?x=2; y=2?");

    let mut list_0_2 = List::new();
    let mut list_2_0 = List::new();
    let mut list_1_0 = List::new();

    loop {
       

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
            setup_screen();
            let mut layout = BoxLayout::new(); 
            layout.add(&lbl_0_0, Size2D{x:0, y: 0});
            layout.add(&lbl_1_1, Size2D{x:1, y: 1});
            layout.add(&lbl_2_2, Size2D{x:2, y: 2});
        
            layout.add(&list_0_2, Size2D{x:0, y: 2});
            layout.add(&list_2_0, Size2D{x:2, y: 0});
            layout.add(&list_1_0, Size2D{x:1, y: 0});
        
            //layout.set_expand_x(0, 1);
            //layout.set_expand_x(1, 1);
            layout.set_expand_y(0, 1);
            layout.set_expand_y(1, 2);
            layout.set_expand_y(2, 3);
            layout.set_expand_x(2, 1);
            //layout.set_expand_y(0, 1);

            layout.draw();
        
            stdout().queue( cursor::MoveTo(80 as u16, 30 as u16) );
            stdout().flush();
       
    }
}
