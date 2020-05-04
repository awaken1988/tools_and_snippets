use std::io::{stdout, Write};
use crossterm::{cursor, terminal, execute, style, event, event::{Event, read, KeyCode}, ExecutableCommand, QueueableCommand};
use crate::imtui::def::{*};

pub struct List {
    data: Vec<Vec<String>>
}

impl List {
    pub fn new() -> Box<dyn Widget> {
        let mut ret = Box::new(List {
            data: Vec::new()
        });

        ret.add_row( vec!["1.1".to_string(), "1.2".to_string(), ]);
        ret.add_row( vec!["2.1".to_string(), "2.2".to_string(), ]);
        ret.add_row( vec!["3.1".to_string(), "3.2".to_string(), ]);
       
        
        return ret;
    }

    pub fn clear(&mut self) {
        self.data.clear();
    }

    pub fn add_row(&mut self, row: Vec<String>) {
        self.data.push(row);
    }

    fn column_count(&self) -> usize {
        let mut count = 0;
        for iRow in &self.data {
            if iRow.len() > count {
                count = iRow.len();
            }
        }

        return count;
    }
}

impl Widget for List {
    fn min_space(&self) -> Size2D {
        return Size2D { 
            x: self.column_count()*5, 
            y: 3 };
    }

    fn expand(&self) -> usize {
        return 0;
    }

    fn draw(&self, aLeftTop: Size2D, aDimension: Size2D)
    {   
        let cols = self.column_count();
        let width_per_col = aDimension.x / cols;

        let mut used_rows = 0;
        for iRow in 0..(self.data.len()) {
            if used_rows >= aDimension.y {
                break;
            }

            for (iColNum, iCol) in self.data[iRow].iter().enumerate() {
                let mut print_copy = iCol.clone();
                let separator = "|".to_string();

                let width_per_col_data = width_per_col - 1;
                if iCol.chars().count() > width_per_col_data {
                    for i in 0..(iCol.chars().count()-width_per_col_data) {
                        print_copy.pop();
                    }
                }
        
                stdout().queue( cursor::MoveTo((aLeftTop.x + width_per_col*iColNum) as u16, (aLeftTop.y + iRow) as u16) );
                stdout().queue( style::Print(print_copy) );
                //stdout().queue( cursor::MoveTo((width_per_col*iColNum-1) as u16, iRow as u16) );
                //stdout().queue( style::Print(separator) );
            }

            used_rows+=1;
        }
    }
}