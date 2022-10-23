use std::io::{stdout, Write};
use crossterm::{cursor, terminal, execute, style, event, event::{Event, read, KeyCode, KeyEvent}, ExecutableCommand, QueueableCommand};
use crate::imtui::def::{*};

const LIST_SCROLLBAR_WIDTH: usize = 1;
const LIST_SELECTION_WIDTH: usize = 1;

pub struct List {
    data: Vec<Vec<String>>,
    pub selection: Option<usize>
}

impl List {
    pub fn new() -> List {
        let mut ret = List {
            data: Vec::new(),
            selection: None,
        };

        //for i_test in (0..23) {
        //    ret.add_row( vec![format!("{} aaa", i_test).to_string(), format!("{} bbb", i_test).to_string(), ]);
        //}
        
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
        if self.data.is_empty() {
            self.selection = None;
        }
        else if let Some(x) = &mut self.selection {
            *x += incr;
            if *x >= self.data.len() {
                *x = self.data.len() - 1;
            }
        } else {
            self.selection = Some(0);
        }
    }

    pub fn decrement_selection(&mut self, incr: usize) {
        if self.data.is_empty() {
            self.selection = None;
        }
        else if let Some(x) = &mut self.selection {
            if *x < incr {
                *x = 0
            }
            else {
                *x -= incr;
            }
           
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
                +LIST_SCROLLBAR_WIDTH, 
            y: 5 };
    }

    fn draw(&self, aLeftTop: Size2D, aDimension: Size2D)
    {   
        let cols = self.column_count();
        let width_per_col = (aDimension.x-LIST_SCROLLBAR_WIDTH-LIST_SELECTION_WIDTH) / cols;

        let half_avail_y = aDimension.y / 2;
        let mut start_rel_y = 0;

        if let Some(sel) = self.selection {
            if sel < half_avail_y {
                start_rel_y = 0;
            }
            else {
                start_rel_y = sel - half_avail_y;
            }
        }

        let mut end_rel_y: usize = *([self.data.len(), start_rel_y+aDimension.y-1 ].iter().min().unwrap());

        

        //draw fields
        for iRow in start_rel_y..(end_rel_y) {
            for (iColNum, iCol) in self.data[iRow].iter().enumerate() {
                let mut print_copy = iCol.clone();
                let separator = "|".to_string();

                let width_per_col_data = width_per_col - 1;
                if iCol.chars().count() > width_per_col_data {
                    for i in 0..(iCol.chars().count()-width_per_col_data) {
                        print_copy.pop();
                    }
                }
        
                if let Some(x) = self.selection {
                    if iRow == x {
                        stdout().queue( style::SetBackgroundColor( style::Color::Green  ) );
                    }
                }
                else {
                    stdout().queue( style::SetBackgroundColor( style::Color::Black ) );
                }

                stdout().queue( cursor::MoveTo( (aLeftTop.x + width_per_col*iColNum) as u16,  (aLeftTop.y + iRow - start_rel_y) as u16 ) );
                stdout().queue( style::Print(print_copy) );
                stdout().queue( style::SetBackgroundColor( style::Color::Black ) );
            }
        }

        //draw scrollbar
        //if let Some(selection) = self.selection {
        //    let scrollbar_y_ppos = (aDimension.y * selection) / self.data.len();
        //    stdout().queue( cursor::MoveTo( (aLeftTop.x + aDimension.x - 1) as u16,  (aLeftTop.y + scrollbar_y_ppos) as u16 ) );
        //    stdout().queue( style::Print("#".to_string()) );
        //}
    }

    fn handle_key(&mut self, aKeyEvent: KeyEvent)
    {
        
    }
}