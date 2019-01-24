use std::collections::HashMap;
use std::path::Path;
use std::fs::read_dir;
use std::fs::DirEntry;

//TODO: what exactly is &mut dyn FnMut :-O ???
//TODO: why  info: Vec<Option<Box<DirEntry>>> doesn't work

struct DiffItem {
    parent: Option<usize>,
    info: Vec<Option<usize>>,
    child: Vec<usize>,
}

struct DiffTree {
    entries: Vec<DiffItem>,
    info: Vec<DirEntry>,
}



fn list_dir(dir: &Path, 
            fun: &mut dyn FnMut(DirEntry, bool) ) 
{
    for iEntry in read_dir(dir).unwrap() {
        let iEntry = iEntry.unwrap();
        let iMeta  = iEntry.metadata().unwrap();
        
        if iMeta.is_dir() {
            fun(iEntry, true);
        }
        else if iMeta.is_file() {
            fun(iEntry, false);
        }
        else {
            println!("[WRN] ignored: {:?}", iEntry);
        }
    }
}

fn compare_dir( dirs: &[&Path], 
                tree: &mut DiffTree,
                parent: Option<usize>) 
{
    let mut included = HashMap::new();
    let start_idx = tree.entries.len();
    for (iDiffIdx, iDir) in dirs.iter().enumerate() {
        list_dir(&iDir, 
        &mut |item,_isdir| { 
            tree.info.push( item );
            let info_idx = tree.info.len()-1;
            let info = &tree.info[info_idx];
            let path_last_part = info.path().file_name().unwrap().to_os_string();

            if !included.contains_key(&path_last_part) {
                included.insert(path_last_part.clone(), tree.entries.len());
                let mut diff_item = DiffItem {  parent: None, 
                                                info: vec![None; dirs.len()], 
                                                child: vec![], };
                tree.entries.push(diff_item);
            }
            
            let idx = match included.get(&path_last_part) {
                Some(x) => x,
                None => panic!("diff_entry should already be included in the HashMap"),
            };

            tree.entries[*idx].info[iDiffIdx] = Some(info_idx); 
        });
    }
    let end_idx = tree.entries.len();
}

fn print_tree(tree: &DiffTree) {
    for i in &tree.entries {
        if let Some(x) = i.parent {
            break;
        }

        println!("#------------------------------------");
        for (jNum, jIdx) in i.info.iter().enumerate() {
            if let Some(jIdx) = jIdx {
                println!("| {}: Result={:?}", jNum, tree.info[*jIdx].path());
            } 
            else {
                 println!("| {}: ", jNum);
            }
        }
        println!("#------------------------------------");

        println!("");
    }
}

fn main() {
    println!("Hello, world!");

    let dirs = [Path::new("testdata/left/"), Path::new("testdata/right/")];
    let mut tree = DiffTree{ entries: vec![], info: vec![] };
    compare_dir( &dirs, &mut tree, None );
    print_tree( &tree );
}
