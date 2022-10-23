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
    border: Size2D,
}

impl TableLayoutCellProperty {
    fn new() -> TableLayoutCellProperty {
        return TableLayoutCellProperty{
            expand: Size2D::zero(),
            border: Size2D::zero(),
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

    pub fn expand(&mut self, expand: &Size2D) -> &mut Self {
        if let Some(last_added) = self.last_added {
            self.cell_properties[last_added.y][last_added.x].expand = expand.clone();
        }
        else {
            panic!("no item to set the expand property; do an add before")
        }

        return self;
    }

    pub fn border(&mut self, expand: &Size2D) -> &mut Self {
        if let Some(last_added) = self.last_added {
            self.cell_properties[last_added.y][last_added.x].border = expand.clone();
        }
        else {
            panic!("no item to set the border property; do an add before")
        }

        return self;
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
        let mut horizontal_expand: Vec<usize> = vec![0; self.max_cols_in_row()];
        let mut vertical_expand: Vec<usize> = vec![0; self.rows.len()];

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
                    let min_space = widget.min_space() + self.cell_properties[i_row][i_col].border;
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

    fn draw_border(&self, aTopLeft: &Size2D, aDimension: &Size2D, aBorder: &Size2D) {
        if aBorder.x > 0 {
            let end_y = aTopLeft.y + aDimension.y + aBorder.y;
            let start_x = aTopLeft.x + aDimension.x ;
            let end_x = start_x + aBorder.x;
            
            
            for i_y in aTopLeft.y..end_y {
                for i_x in start_x..end_x {
                    stdout().queue( cursor::MoveTo( i_x as u16,  i_y as u16 ) );
                    stdout().queue( style::Print("|".to_string()) );
                }
            }
        }
        if aBorder.y > 0 {
            let start_y = aTopLeft.y + aDimension.y;
            let end_y = start_y + aBorder.y;
            let start_x = aTopLeft.x;
            let len_x = aDimension.x;

            for i_y in start_y..end_y {
                let mut separator = String::with_capacity(len_x);
                for i_len in 0..len_x {
                    separator.push('-');
                }
                stdout().queue( cursor::MoveTo( start_x as u16,  i_y as u16 ) );
                stdout().queue( style::Print(separator)  );
            }
        }
    }
}

impl<'a> Widget for TableLayout<'a> {
    fn min_space(&self) -> Size2D {
        return Size2D{x: 0, y: 0};  //TODO: later return the real size
    }
    
    fn draw(&self, aLeftTop: Size2D, aDimension: Size2D) {
        let mut cell_size = self.collect_min_size();
        let mut cell_size = self.expand_cells(aDimension, &cell_size);

        let mut vertical_used = 0;
        for i_row in 0..self.rows.len() {   
            let mut horizontal_used = 0;
            for i_col in 0..self.rows[i_row].len() {
                let mut horizontal_use = cell_size.horizontal_cell_width[i_col];
                let mut vertical_use = cell_size.vertical_cell_height[i_row];
                
                let border_space = self.cell_properties[i_row][i_col].border; 

                let pos = Size2D{
                    x: horizontal_used, 
                    y: vertical_used };

                let dimension = Size2D{
                    x: horizontal_use - border_space.x, 
                    y: vertical_use - border_space.y };

                

                //draw widget
                if let Some(widget) = &self.rows[i_row][i_col] {
                    widget.draw(pos, dimension);
                }

                //draw border
                self.draw_border(&pos, &dimension, &border_space);

                horizontal_used += cell_size.horizontal_cell_width[i_col];
            }
            vertical_used += cell_size.vertical_cell_height[i_row];
        }  
        
        stdout().flush();
    }

    fn handle_key(&mut self, aKeyEvent: KeyEvent) {

    }
}

