use std::collections::HashMap;
use std::collections::BTreeSet;
use std::path::Path;
use std::path::PathBuf;
use std::fs::read_dir;
use std::fs::DirEntry;
use std::fs::metadata;
use std::rc::Rc;
use std::cell::RefCell;
use std::collections::VecDeque;

#[derive(Debug)]
pub struct ImplDiffItem {
    parent: Option<usize>,
    child: Vec<usize>,
    path: Vec<Option<PathBuf>>,
}

#[derive(Debug)]
pub struct DiffItem {
    pub flat_data: Rc<Vec<ImplDiffItem>>,
    pub idx: Option<usize>,
}

impl DiffItem {
    pub fn test_get_iterator(&self) -> DiffItemIterator {
        let mut ret = DiffItemIterator{ state: vec![], flat_data: Rc::clone(&self.flat_data)};

        if !self.flat_data.is_empty() {
            ret.state.push( vec![0, ]);
        }    

        return ret;
    }

    pub fn depth(&self) -> usize {
        let mut ret = 0;
        let mut idx = self.idx;

        loop {
            if let Some(idx_num) = idx {
                ret += 1;
                idx = self.flat_data[idx_num].parent;
                continue;
            }

            return ret;
        }
    }
}

pub fn diff_dirs(dirs: &Vec<Option<PathBuf>>) -> DiffItem
{
    let mut flat_data: Vec<ImplDiffItem> = Vec::new();
    let mut remain_dirs: VecDeque<usize> = VecDeque::new();

    //create the first entry
    flat_data.push( ImplDiffItem { parent: None, child: Vec::new(), path: Vec::clone(dirs)});
    remain_dirs.push_back(0);

    while !remain_dirs.is_empty() {
        let mut data_end_idx = flat_data.len();
        let mut curr_idx = remain_dirs.pop_front().unwrap();
        let mut curr_node =  &mut flat_data[curr_idx]; 
       
        let (mut sub_dirs,mut  dirs_indices) = impl_list_subdir(&curr_node.path);

        //update indices
        for (i_sub_idx,mut i_sub_dir) in sub_dirs.iter_mut().enumerate() {
            i_sub_dir.parent = Some(curr_idx);
            curr_node.child.push(data_end_idx + i_sub_idx);
        }

        //append DirItem Vec
        flat_data.append(&mut sub_dirs);

        //enqueue new subdirs
        for i_sub in &dirs_indices {
            remain_dirs.push_back(*i_sub + data_end_idx)
        }
    }

    return DiffItem { flat_data: Rc::new(flat_data), idx: Some(0) };
}

pub struct DiffItemIterator {
    state:  Vec<Vec<usize>>,
    flat_data: Rc<Vec<ImplDiffItem>>,
}

impl Iterator for DiffItemIterator {
    type Item = DiffItem;

    fn next(&mut self) -> Option<DiffItem> 
    {
        loop {
            if self.state.is_empty() { 
                return None; }

            if self.state.last().unwrap().is_empty() {
                self.state.pop();
                continue; }

            let idx = self.state.last_mut().unwrap().pop().unwrap();

            self.state.push( self.flat_data[idx].child.clone() );

            return Some(DiffItem {  flat_data: Rc::clone(&self.flat_data), 
                                    idx: Some(idx) });
        }
    }
}


fn impl_list_subdir(dirs: &Vec<Option<PathBuf>>) -> (Vec<ImplDiffItem>, BTreeSet<usize>)
{
    let mut ret: Vec<ImplDiffItem> = Vec::new();
    let mut used = HashMap::new();
    let mut dirs_indices = BTreeSet::new();

    for (i_idx, i_sib) in dirs.iter().enumerate() {
        if let Some(i_sib) = i_sib {
            list_dir(i_sib, &mut |item,_isdir| { 
                let curr_path = item.path().to_path_buf();
                let curr_last_part = curr_path.file_name().unwrap().to_os_string();

                if !used.contains_key(&curr_last_part) {
                    ret.push( ImplDiffItem { parent: None, child: Vec::new(), path: vec![None; dirs.len()]});
                    used.insert(curr_last_part.clone(), ret.len()-1);
                }

                let idx = match used.get(&curr_last_part) {
                    Some(x) => x,
                    None => panic!("diff_entry should already be included in the HashMap"),
                };

                ret[*idx].path[i_idx] = Some(curr_path.clone());
                dirs_indices.insert(*idx);
            });
        } 
    }

    return (ret, dirs_indices);
}



//fn impl_expand_dirs(dirs:   &Vec<Option<PathBuf>>, 
//                    tree:   Rc<RefCell<flat_data_t>>,
//                    parent: usize) 
//{
//    let real_parent = parent;
//
//    //iterate over all DirEntries and add it to tree
//    let mut included = HashMap::new();  //each dir should only get one numer; 
//                                        //TODO: is HashMap and colission?
//    let mut sub_dirs = BTreeSet::new();
//
//    //list all files,dirs 
//    let start_idx = tree.borrow().len();
//    for (i_diff_idx, i_dir) in dirs.iter().enumerate() {
//        let mut mut_tree = tree.borrow_mut();
//
//        if let Some(i_dir)  = i_dir {
//              list_dir(&i_dir, 
//                &mut |item,_isdir| { 
//                    let curr_path = item.path().to_path_buf();
//                    let curr_last_part = curr_path.file_name().unwrap().to_os_string();
//
//                    if !included.contains_key(&curr_last_part) {
//                        included.insert(curr_last_part.clone(), mut_tree.len());
//                        let mut diff_item = DiffItem {  parent: Some(real_parent), 
//                                                        path: vec![None; dirs.len()], 
//                                                        child: vec![],
//                                                        flat_data: Rc::clone(&tree)  };
//                        mut_tree.push(diff_item);
//                        let curr_idx = mut_tree.len()-1;
//                        mut_tree[real_parent].child.push(curr_idx); 
//                    }
//                    
//                    let idx = match included.get(&curr_last_part) {
//                        Some(x) => x,
//                        None => panic!("diff_entry should already be included in the HashMap"),
//                    };
//
//                    mut_tree[*idx].path[i_diff_idx] = Some(curr_path.clone()); 
//
//                    if curr_path.metadata().unwrap().is_dir() {
//                        sub_dirs.insert(*idx);
//                    }
//                });
//        }
//    }
//    //let end_idx = tree.borrow().len();
//
//    //add all subdirs
//    for i_dir in sub_dirs {
//        let mut dirs_next: Vec<Option<PathBuf>> = Vec::new();
//
//        for i_ss in &tree.borrow()[i_dir].path {
//            if let Some(x) = i_ss { 
//                dirs_next.push(Some(x.clone()));
//            }
//            else { 
//                dirs_next.push(None);
//            }
//        }
//
//        impl_expand_dirs(&dirs_next, Rc::clone(&tree), i_dir );
//    } 
//}
//
////pub fn get_diff_depth<'a>(diff_item: &'a DiffItem) -> i32 {
////    let mut depth = 0i32;
////    
////    let mut item = diff_item;
////
////    while let Some(x) = item.parent {
////        depth+=1;
////        item = &diff_item.flat_data[x];
////    }
////    
////    return depth;
////}
////
////
//pub fn print_tree_flat(tree: &DiffTree) {
//    for (i_idx, i) in tree.flat_data.borrow().iter().enumerate() {
//        println!("{} {:?}",i_idx, i.path);
//        println!("    parent={:?}", i.parent);
//        println!("    childs={:?}", i.child);
//    }
//}
////
////pub fn print_tree(tree: &DiffTree, parent: usize, depth: usize) {
////
////    let mut dept_str = String::new();
////    for i in 0..(depth*4) {
////        dept_str += " ";
////    }
////
////    for i_child in &tree.entries[parent].child {
////        let curr_entry = &tree.entries[*i_child].path;
////        
////        println!("{}#------------------------------------", dept_str);
////        for (j_num, j_path) in curr_entry.iter().enumerate() {
////            if let Some(j_path) = j_path {
////                println!("{}| {}: Result={:?}", dept_str, j_num, j_path.file_name());
////            } 
////            else {
////                println!("{}| {}: ", dept_str, j_num);
////            }
////        }
////        println!("");
////
////        print_tree(tree, *i_child, depth+1);
////
////    }
////}
////
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