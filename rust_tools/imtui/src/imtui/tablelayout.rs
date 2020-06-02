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

        self.rows[position.x][position.y] = Some(widget);
    }

    pub fn is_cell_free(&self, position: Size2D) -> bool {
        if (self.rows.len().overflowing_sub(position.x+1)).1 {
            return true;
        }
        else if (self.rows[0].len().overflowing_sub(position.y+1)).1 {
            return true;
        }
        else if let Some(_) = &self.rows[position.x][position.y] {
            return false;
        }
        else {
            return true;
        }
    }

    pub fn size(&self) -> Size2D {
        return Size2D{x: self.rows.len(), y: self.rows.get(0).map_or(0, |v| v.len())}
    }

    //NOTE: cache the size in future?
    fn make_table_fit(&mut self, position: Size2D) {
        if let Some(result) = (position.x+1).checked_sub(self.rows.len()) {
            for i_rows in 0..result {
                self.rows.push( Vec::new() )
            }
        }

        let max_cols = self.max_cols_in_row();
        for i_row in &mut self.rows {
            if let Some(result) = (position.y+1).checked_sub(i_row.len()) {
                for i_col_fill in 0..result {
                    i_row.push(None);
                }
            }
        }

    }

    fn max_cols_in_row(&self) -> usize {
        return self.rows.iter().map(|x| x.len()).max().unwrap_or(0);
    }
        
}

