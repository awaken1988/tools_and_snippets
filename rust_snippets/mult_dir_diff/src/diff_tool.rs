use std::collections::HashMap;
use std::collections::BTreeSet;
use std::path::Path;
use std::path::PathBuf;
use std::fs::read_dir;
use std::fs::DirEntry;
use std::fs::metadata;
   
#[derive(Debug)]
pub struct DiffItem {
    pub parent: Option<usize>,
    pub child: Vec<usize>,
    pub path: Vec<Option<PathBuf>>,
}

#[derive(Debug)]
pub struct DiffTree {
    pub entries: Vec<DiffItem>,
}

//impl DiffTree {
//    fn get_iterator(&mut self ) -> DiffTreeIterator {
//       
//    }
//}

pub fn get_iterator(tree: &DiffTree) -> DiffTreeIterator {

    let root_entry = vec!(IterState { child: &tree.entries[0].child, idx: 0 });

    let a = DiffTreeIterator {state: root_entry, tree: tree };    
    a
}

//---------------------------------------
// iter 
//---------------------------------------
//TODO: IterState could be implemented with Vec<usize> Iterators -> would be cleaner
struct IterState<'a> {
    child: &'a Vec<usize>,
    idx: usize,
}

pub struct DiffTreeIterator<'a> {
    state: Vec<IterState<'a>>,
    tree: &'a DiffTree,
}

impl<'a> Iterator for DiffTreeIterator<'a> {
    type Item = &'a DiffItem;
    fn next(&mut self) -> Option<&'a DiffItem> {

        loop {
            let idx;
            if self.state.is_empty() { return None; }
            let curr_idx = self.state.len()-1;
            {
                
              
                let curr_state = &mut self.state[curr_idx];

                if curr_state.idx >= curr_state.child.len() {
                    self.state.pop();
                    continue;
                }

                 idx = curr_state.idx;
                 curr_state.idx = idx + 1;
            }
           
            if self.tree.entries[self.state[curr_idx].child[idx]].child.len() > 0 {
                self.state.push( IterState { child: &self.tree.entries[self.state[curr_idx].child[idx]].child, idx: 0 } )
            }

            return Some(&self.tree.entries[self.state[curr_idx].child[idx]]);
        }
    }
}

pub fn expand_dirs( dirs: &Vec<Option<PathBuf>>, 
                    tree: &mut DiffTree,
                    parent: Option<usize>) 
{
    let real_parent = if let Some(x) = parent {
        x
    }
    else { 
        let mut root_item = DiffItem {  parent: parent, path: dirs.clone(), child: vec![], };
        tree.entries.push(root_item);
        tree.entries.len()-1 
    };

    //iterate over all DirEntries and add it to tree
    let mut included = HashMap::new();  //each dir should only get one numer; 
                                        //TODO: is HashMap and colission?
    let mut sub_dirs = BTreeSet::new();

    //list all files,dirs 
    let start_idx = tree.entries.len();
    for (i_diff_idx, i_dir) in dirs.iter().enumerate() {
        if let Some(i_dir)  = i_dir {
              list_dir(&i_dir, 
                &mut |item,_isdir| { 
                    let curr_path = item.path().to_path_buf();
                    let curr_last_part = curr_path.file_name().unwrap().to_os_string();

                    if !included.contains_key(&curr_last_part) {
                        included.insert(curr_last_part.clone(), tree.entries.len());
                        let mut diff_item = DiffItem {  parent: Some(real_parent), 
                                                        path: vec![None; dirs.len()], 
                                                        child: vec![], };
                        tree.entries.push(diff_item);
                        let curr_idx = tree.entries.len()-1;
                        tree.entries[real_parent].child.push(curr_idx); 
                    }
                    
                    let idx = match included.get(&curr_last_part) {
                        Some(x) => x,
                        None => panic!("diff_entry should already be included in the HashMap"),
                    };

                    tree.entries[*idx].path[i_diff_idx] = Some(curr_path.clone()); 

                    if curr_path.metadata().unwrap().is_dir() {
                        sub_dirs.insert(*idx);
                    }
                });
        }
    }
    let end_idx = tree.entries.len();

    //add all subdirs
    for i_dir in sub_dirs {
        expand_dirs( &tree.entries[i_dir].path.clone(), tree, Some(i_dir) );
    } 
}

pub fn get_diff_depth<'a>(tree: &'a DiffTree, item: &'a DiffItem) -> i32 {
    let mut depth = 0i32;
    
    let mut item = item;

    while let Some(x) = item.parent {
        depth+=1;
        item = &tree.entries[x];
    }
    
    return depth;
}


pub fn print_tree_flat(tree: &DiffTree) {
    for (i_idx, i) in tree.entries.iter().enumerate() {
        println!("{} {:?}",i_idx, i);
    }
}

pub fn print_tree(tree: &DiffTree, parent: usize, depth: usize) {

    let mut dept_str = String::new();
    for i in 0..(depth*4) {
        dept_str += " ";
    }

    for i_child in &tree.entries[parent].child {
        let curr_entry = &tree.entries[*i_child].path;
        
        println!("{}#------------------------------------", dept_str);
        for (j_num, j_path) in curr_entry.iter().enumerate() {
            if let Some(j_path) = j_path {
                println!("{}| {}: Result={:?}", dept_str, j_num, j_path.file_name());
            } 
            else {
                println!("{}| {}: ", dept_str, j_num);
            }
        }
        println!("");

        print_tree(tree, *i_child, depth+1);

    }
}

fn list_dir(dir: &Path, 
            fun: &mut dyn FnMut(DirEntry, bool) ) 
{
    let dir_iter = read_dir(dir);

    if let Ok(dir_iter) = dir_iter {
        for i_entry in dir_iter {
            let i_entry = i_entry.unwrap();
            let i_meta  = i_entry.metadata().unwrap();
            
            if i_meta.is_dir() {
                fun(i_entry, true);
            }
            else if i_meta.is_file() {
                fun(i_entry, false);
            }
            else {
                println!("[WRN] ignored: {:?}", i_entry);
            }
        }
    }
    else if let Err(e) = dir_iter {
        println!("list_dir: {:?}", e);
    }


}