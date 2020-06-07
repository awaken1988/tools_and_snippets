use std::io::{stdout, Write};
use crossterm::{cursor, terminal, execute, style, event, event::{Event, read, KeyCode, KeyEvent}, ExecutableCommand, QueueableCommand};
use crate::imtui::def::{*};
use std::cell::RefCell;
use std::collections::HashMap;


pub struct TableLayout<'a> {
    rows: Vec<Vec<Option<&'a mut dyn Widget>>>,
    
    last_added: Option<Size2D>,

    cell_properties: Vec<Vec<TableLayoutCellProperty>>,
}

#[derive(Clone)]
struct TableLayoutCalculation {
    horizontal_cell_width: Vec<usize>,  //index is y
    horizontal_sum: usize,
    
    vertical_cell_height: Vec<usize>,   //index is x
    vertical_sum: usize,
    
}


struct TableLayoutCellProperty {
    expand: Size2D,
}

impl TableLayoutCellProperty {
    fn new() -> TableLayoutCellProperty {
        return TableLayoutCellProperty{
            expand: Size2D::zero(),
        }
    }
}

impl<'a> TableLayout<'a> {
    pub fn new() -> TableLayout<'a> {
        return TableLayout {
            rows: Vec::new(),
            last_added: None,
            cell_properties: Vec::new(),
        };
    }

    pub fn add(&mut self, widget: &'a mut dyn Widget, position: Size2D) -> &mut Self {
        if !self.is_cell_free(position) {
            panic!("cell is not free");
        }

        self.make_table_fit(position);

        self.rows[position.y][position.x] = Some(widget);
        self.last_added = Some(position);  //set the last position set

        return self;
    }

    pub fn expand(&mut self, expand: Size2D) {
        if let Some(last_added) = self.last_added {
            self.cell_properties[last_added.y][last_added.x].expand = expand;
        }
        else {
            panic!("no item to set the expand property; do an add before")
        }
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
                self.rows.push( Vec::new() );
                self.cell_properties.push( Vec::new() );
            }
        }

        let mut max_cols = self.max_cols_in_row();
        if (position.x+1) > max_cols {
            max_cols = (position.x+1); 
        }

        for i_row in 0..self.rows.len() {
            if max_cols >= self.rows[i_row].len() {
                for i in 0..(max_cols-self.rows[i_row].len()) {
                    self.rows[i_row].push(  None );
                    self.cell_properties[i_row].push( TableLayoutCellProperty::new() );
                }
            } 
        }
    }

    fn max_cols_in_row(&self) -> usize {
        return self.rows.iter().map(|x| x.len()).max().unwrap_or(0);
    }

    fn expand_cells(&self, dimension: Size2D, table_calc: &TableLayoutCalculation) -> TableLayoutCalculation {
        let mut ret = table_calc.clone();

        let mut horizontal_delta = dimension.x - table_calc.horizontal_sum;
        let mut vertical_delta = dimension.y - table_calc.vertical_sum;

        //sum
        let mut horizontal_expand: Vec<usize> = vec![0, self.max_cols_in_row()];
        let mut vertical_expand: Vec<usize> = vec![0, self.rows.len()];

        for i_row in 0..self.rows.len() {
            for i_col in 0..self.max_cols_in_row() {
                let col = &self.cell_properties[i_row][i_col];

                if col.expand.x > horizontal_expand[i_col] {
                    horizontal_expand[i_col] = col.expand.x;
                }

                if col.expand.y > vertical_expand[i_row] {
                    vertical_expand[i_row] = col.expand.y;
                }
            }
        }

        let horizontal_expand_count: usize = horizontal_expand.iter().sum();
        let vertical_expand_count: usize = vertical_expand.iter().sum();

        // distribute free space
        if horizontal_expand_count > 0 {
            for i_col in 0..self.max_cols_in_row() {
                let val = (horizontal_delta * horizontal_expand[i_col]) / horizontal_expand_count;
                ret.horizontal_cell_width[i_col] += val;
                ret.horizontal_sum += val
            }
        }
        if vertical_expand_count > 0 {
            for i_row in 0..self.rows.len() {
                let val = (vertical_delta * vertical_expand[i_row]) / vertical_expand_count;
                ret.vertical_cell_height[i_row] += val;
                ret.vertical_sum += val;
            }
        }
        
        return ret;
    }

    fn collect_min_size(&self) -> TableLayoutCalculation {
        if self.is_empty() {
            return TableLayoutCalculation{
                horizontal_cell_width: Vec::new(),
                horizontal_sum: 0,
                vertical_cell_height: Vec::new(),
                vertical_sum: 0,
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

        let horizontal_sum = horizontal_cell_width.iter().sum();
        let vertical_sum = vertical_cell_height.iter().sum();

        return TableLayoutCalculation{
            horizontal_cell_width: horizontal_cell_width,
            horizontal_sum: horizontal_sum,
            vertical_cell_height: vertical_cell_height,
            vertical_sum: vertical_sum,
        };
    }

}

impl<'a> Widget for TableLayout<'a> {
    fn min_space(&self) -> Size2D {
        return Size2D{x: 0, y: 0};  //TODO: later return the real size
    }
    
    fn draw(&self, aLeftTop: Size2D, aDimension: Size2D) {
        let mut cell_size = self.collect_min_size();
        let mut cell_size_expanded = self.expand_cells(aDimension, &cell_size);

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

