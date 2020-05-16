use std::io::{stdout, Write};
use crossterm::{cursor, terminal, execute, style, event, event::{Event, read, KeyCode}, ExecutableCommand, QueueableCommand};
use crate::imtui::def::{*};

const TABLELAYOUT_SCROLLBAR_WIDTH: usize = 1;

pub struct List {
    data: Vec<Vec<String>>,
    selection: Option<usize>
}

impl List {
    pub fn new() -> Box<dyn Widget> {
        let mut ret = Box::new(List {
            data: Vec::new(),
            selection: None,
        });

        ret.add_row( vec!["1.1".to_string(), "1.2".to_string(), ]);
        ret.add_row( vec!["2.1".to_string(), "2.2".to_string(), ]);
        ret.add_row( vec!["3.1".to_string(), "3.2".to_string(), ]);
        ret.add_row( vec!["4.1".to_string(), "4.2".to_string(), ]);
        ret.add_row( vec!["5.1".to_string(), "5.2".to_string(), ]);
        ret.add_row( vec!["6.1".to_string(), "6.2".to_string(), ]);
        ret.add_row( vec!["7.1".to_string(), "7.2".to_string(), ]);
        ret.add_row( vec!["8.1".to_string(), "8.2".to_string(), ]);
        ret.add_row( vec!["9.1".to_string(), "9.2".to_string(), ]);
        ret.add_row( vec!["a.1".to_string(), "a.2".to_string(), ]);
        ret.add_row( vec!["b.1".to_string(), "b.2".to_string(), ]);
       
        
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

    pub fn increment_selection(&mut self, incr: usize) {
        if let Some(x) = &mut self.selection {
            *x += incr;
        } else {
            self.selection = Some(0);
        }
    }

    pub fn decrement_selection(&mut self, incr: usize) {
        if let Some(x) = &mut self.selection {
            *x -= incr;
        } else {
            self.selection = Some(0);
        }
    }
}

impl Widget for List {
    fn min_space(&self) -> Size2D {
        return Size2D { 
            x: self.column_count()*5
                +1 
                +TABLELAYOUT_SCROLLBAR_WIDTH, 
            y: 5 };
    }

    fn expand(&self) -> usize {
        return 0;
    }

    fn draw(&self, aLeftTop: Size2D, aDimension: Size2D)
    {   
        let cols = self.column_count();
        let width_per_col = (aDimension.x-TABLELAYOUT_SCROLLBAR_WIDTH) / cols;

        //draw fields
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
        
                stdout().queue( cursor::MoveTo( (aLeftTop.x + width_per_col*iColNum) as u16,  (aLeftTop.y + iRow) as u16 ) );
                stdout().queue( style::Print(print_copy) );
            }

            used_rows+=1;
        }

        //draw scrollbar
        if let Some(selection) = self.selection {
            let scrollbar_y_ppos = (aDimension.y * selection) / self.data.len();
            stdout().queue( cursor::MoveTo( (aLeftTop.x + aDimension.x - 1) as u16,  (aLeftTop.y + scrollbar_y_ppos) as u16 ) );
            stdout().queue( style::Print("-".to_string()) );
        }
        
    }
}