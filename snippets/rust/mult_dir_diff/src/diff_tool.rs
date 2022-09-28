use std::collections::HashMap;
use std::collections::BTreeSet;
use std::collections::BTreeMap;
use std::path::Path;
use std::path::PathBuf;
use std::fs::read_dir;
use std::fs::DirEntry;
use std::fs::metadata;
use std::rc::Rc;
use std::cell::RefCell;
use std::collections::VecDeque;
use std::fs;
use std::cmp::min;
use std::cmp::max;


#[derive(Debug)]
pub struct DiffItemInfo {
    pub parent: Option<usize>,
    pub child: Vec<usize>,
    pub path: Vec<Option<PathBuf>>,
    pub relative_path: Option<PathBuf>,
}

#[derive(Debug)]
pub struct DiffItem {
    pub flat_data: Rc<Vec<DiffItemInfo>>,
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

    pub fn info(&self) -> &DiffItemInfo {
        return &self.flat_data[self.idx.unwrap()]
    }

    pub fn name(&self) -> Option<PathBuf> {
        for i_path in &self.info().path {
            if let Some(i_path) = i_path {
                return Some(PathBuf::from( i_path.file_name().unwrap() ) );
            }
        }

        return None;
    }

     pub fn relative_path(&self) -> PathBuf {
        return self.info().relative_path.as_ref().unwrap().clone();
    }

    pub fn siblings(&self) -> usize {
        return self.info().path.len();
    }

    pub fn find_duplicates(&self) -> Vec<Option<usize>> {
        let mut ret: Vec<Option<usize>> = Vec::new();
        let mut next: usize = 0;

        //assume all items are different
        //TODO: use some fancy functional programming iterators
        for i_ret in 0..self.siblings() {
            if None == self.info().path[i_ret] { 
                ret.push(None)
            } else {
                ret.push(Some(next));
                next+=1;
            } 
        }

        for (o_idx, o_val) in self.info().path.iter().enumerate() {
            let o_val = if let Some(x) = o_val { x } else {continue};
            for (i_idx, i_val) in self.info().path.iter().enumerate() {
                if o_idx == i_idx { continue };
                let i_val = if let Some(x) = i_val { x } else {continue};
               
                if o_val.is_dir() && i_val.is_dir() {  
                    ret[ max(o_idx, i_idx) ] = ret [min(o_idx, i_idx)];
                    ret[ min(o_idx, i_idx) ] = ret [min(o_idx, i_idx)];
                    continue;
                }
                
                if !o_val.is_file() && !i_val.is_file() {  
                    //TODO: get a warning and handle these types
                    continue;
                }

                //get metadata
                let o_meta = if let Ok(x) = o_val.metadata() { x } else { continue; }; //TODO: when continue inform that we cannot get the info
                let i_meta = if let Ok(x) = i_val.metadata() { x } else { continue; }; //TODO: when continue inform that we cannot get the info

                if o_meta.len() != i_meta.len() {
                    continue;
                }

                ret[ max(o_idx, i_idx) ] = ret [min(o_idx, i_idx)];
                ret[ min(o_idx, i_idx) ] = ret [min(o_idx, i_idx)];
            }
        }

        return ret;
    }

}

impl IntoIterator for DiffItem {
    type Item = DiffItem;
    type IntoIter = DiffItemIterator;

    fn into_iter(self) -> Self::IntoIter {
        return self.test_get_iterator();
    }
}

pub fn diff_dirs(dirs: &Vec<Option<PathBuf>>) -> DiffItem
{
    let mut flat_data: Vec<DiffItemInfo> = Vec::new();
    let mut remain_dirs: VecDeque<usize> = VecDeque::new();

    //create the first entry
    flat_data.push( DiffItemInfo { parent: None, child: Vec::new(), path: Vec::clone(dirs), relative_path: None, });
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

    //update relative_path
    impl_relative_path(&mut flat_data);

    return DiffItem { flat_data: Rc::new(flat_data), idx: Some(0) };
}

fn impl_relative_path(flat_data: &mut Vec<DiffItemInfo>)
{
    if flat_data.is_empty() { return; }

    let prefix = flat_data[0].path.clone();

    for i_node in flat_data.iter_mut() {
        for (i_idx, i_path) in i_node.path.iter().enumerate() {
            match i_path {
                Some(x) =>  {
                    let prefix: &PathBuf = prefix[i_idx].as_ref().unwrap();
                    i_node.relative_path = Some(x.strip_prefix(prefix).unwrap().to_path_buf());
                    break;
                },            
                None => { },     
            };
        }
    }
}

pub struct DiffItemIterator {
    state:  Vec<Vec<usize>>,
    flat_data: Rc<Vec<DiffItemInfo>>,
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


fn impl_list_subdir(dirs: &Vec<Option<PathBuf>>) -> (Vec<DiffItemInfo>, BTreeSet<usize>)
{
    let mut ret: Vec<DiffItemInfo> = Vec::new();
    let mut used = HashMap::new();
    let mut dirs_indices = BTreeSet::new();

    for (i_idx, i_sib) in dirs.iter().enumerate() {
        if let Some(i_sib) = i_sib {
            list_dir(i_sib, &mut |item,_isdir| { 
                let curr_path = item.path().to_path_buf();
                let curr_last_part = curr_path.file_name().unwrap().to_os_string();

                if !used.contains_key(&curr_last_part) {
                    ret.push( DiffItemInfo { parent: None, child: Vec::new(), path: vec![None; dirs.len()], relative_path: None, });
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
                //println!("[WRN] ignored: {:?}", i_entry);
            }
        }
    }
    else if let Err(e) = dir_iter {
        //println!("list_dir: {:?}", e);
    }
}