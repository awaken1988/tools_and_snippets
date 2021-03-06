use std::io::{stdout, Write};
use crossterm::{cursor, terminal, execute, style, event, event::{Event, read, KeyCode, KeyEvent}, ExecutableCommand, QueueableCommand};
use crate::imtui::def::{*};

pub struct Label {
    text: String,
}

impl Label {
    pub fn new(text: &str) -> Label {
        let ret = Label {
            text: text.to_string(), 
        };

        return ret;
    }
}

impl Widget for Label {
    fn min_space(&self) -> Size2D {
        return Size2D { 
            x: self.text.chars().count(), 
            y: 1 };
    }

    fn draw(&self, aLeftTop: Size2D, aDimension: Size2D)
    {
        let mut print_copy = self.text.clone();

        if self.text.chars().count() > aDimension.x {
            for i in 0..(self.text.chars().count()-aDimension.x) {
                print_copy.pop();
            }
        }

        stdout().queue( cursor::MoveTo(aLeftTop.x as u16, aLeftTop.y as u16) );
        stdout().queue( style::Print(print_copy) );
    }

    fn handle_key(&mut self, aKeyEvent: KeyEvent)
    {
            
    }
}