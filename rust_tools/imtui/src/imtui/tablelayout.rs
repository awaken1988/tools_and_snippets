use std::io::{stdout, Write};
use crossterm::{cursor, terminal, execute, style, event, event::{Event, read, KeyCode, KeyEvent}, ExecutableCommand, QueueableCommand};
use crate::imtui::def::{*};
use std::cell::RefCell;


pub struct TableLayoutItem<'a> {
    widget: &'a mut dyn Widget,
}

pub struct TableLayout<'a> {
    rows: Vec<Vec<Option<&'a mut dyn Widget>>>
}

struct TableLayoutCalculation {
    horizontal_cell_width: Vec<usize>,  //index is y
    vertical_cell_height: Vec<usize>,   //index is x
}

impl<'a> TableLayout<'a> {
    pub fn new() -> TableLayout<'a> {
        return TableLayout {
            rows: Vec::new()
        };
    }

    pub fn add(&mut self, widget: &'a mut dyn Widget, position: Size2D) {
        if !self.is_cell_free(position) {
            panic!("cell is not free");
        }

        self.make_table_fit(position);

        self.rows[position.y][position.x] = Some(widget);
    }

    pub fn is_cell_free(&self, position: Size2D) -> bool {
        if position.y >= self.rows.len() {
            return true;
        }
        if position.x >= self.rows[0].len() {
            return true
        }
        else if let Some(_) = &self.rows[position.y][position.x] {
            return false;
        }
        else {
            return true;
        }
    }

    pub fn size(&self) -> Size2D {
        if self.is_empty() {
            return Size2D{x: 0, y: 0}
        }
        else {
            return Size2D{x: self.rows.get(0).map_or(0, |v| v.len()), y: self.rows.len()};
        }
    }

    pub fn is_empty(&self) -> bool {
        if self.rows.is_empty() {
            return true;
        }
        else if self.rows[0].is_empty() {
            return true;    
        }

        return false;
    }

    //NOTE: cache the size in future?
    fn make_table_fit(&mut self, position: Size2D) {
        if let Some(result) = (position.y+1).checked_sub(self.rows.len()) {
            for i_rows in 0..result {
                self.rows.push( Vec::new() )
            }
        }

        let mut max_cols = self.max_cols_in_row();
        if (position.x+1) > max_cols {
            max_cols = (position.x+1); 
        }

        for i_row in &mut self.rows {
            if max_cols >= i_row.len() {
                for i in 0..(max_cols-i_row.len()) {
                    i_row.push(  None );
                }
            } 
        }
    }

    fn max_cols_in_row(&self) -> usize {
        return self.rows.iter().map(|x| x.len()).max().unwrap_or(0);
    }

    fn collect_min_size(&self) -> TableLayoutCalculation {
        if self.is_empty() {
            return TableLayoutCalculation{
                horizontal_cell_width: Vec::new(),
                vertical_cell_height: Vec::new(),
            };
        }

        let mut horizontal_cell_width = vec![0; self.rows[0].len()];
        let mut vertical_cell_height = vec![0; self.rows.len()];

        //get min size
        for i_row in 0..self.rows.len() {   
            for i_col in 0..self.rows[i_row].len() {
                if let Some(widget) = &self.rows[i_row][i_col] {
                    let min_space = widget.min_space();
                    if horizontal_cell_width[i_col] < min_space.x {
                        horizontal_cell_width[i_col] = min_space.x;
                    }
                    if vertical_cell_height[i_row] < min_space.y {
                        vertical_cell_height[i_row] = min_space.y;
                    }
                }
            }
        }

        return TableLayoutCalculation{
            horizontal_cell_width: horizontal_cell_width,
            vertical_cell_height: vertical_cell_height,
        };
    }

}

impl<'a> Widget for TableLayout<'a> {
    fn min_space(&self) -> Size2D {
        return Size2D{x: 0, y: 0};  //TODO: later return the real size
    }
    
    fn draw(&self, aLeftTop: Size2D, aDimension: Size2D) {
        let mut cell_size = self.collect_min_size();

        let mut vertical_used = 0;
        for i_row in 0..self.rows.len() {   
            let mut horizontal_used = 0;
            for i_col in 0..self.rows[i_row].len() {
                let mut horizontal_use = cell_size.horizontal_cell_width[i_col];
                let mut vertical_use = cell_size.vertical_cell_height[i_row];
                if let Some(widget) = &self.rows[i_row][i_col] {
                    widget.draw(
                        Size2D{x: horizontal_used, y: vertical_used},
                        Size2D{x: horizontal_use, y: vertical_use});
                }
                horizontal_used += cell_size.horizontal_cell_width[i_col];
            }
            vertical_used += cell_size.vertical_cell_height[i_row];
        }  
        
        stdout().flush();
    }

    fn handle_key(&mut self, aKeyEvent: KeyEvent) {

    }
}

