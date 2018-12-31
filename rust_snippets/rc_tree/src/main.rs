#![allow(warnings)]

use std::rc::Rc;
use std::rc::Weak;
use std::cell::RefCell;
use std::fmt;

struct MyNode {
    name: String,
    childs: Vec<Rc<RefCell<MyNode>>>,
    parent: Option<Weak<RefCell<MyNode>>>,
}

fn fill_tree(parent: &Rc<RefCell<MyNode>>, depth: i32) {
    if depth < 0 {
        return;
    }

    for iChild in 0..2 {
        let mut root: Rc<RefCell<MyNode>>  = Rc::new( RefCell::new( MyNode {
            name: format!("{}:{}", depth, iChild).to_string(),
            childs: vec![],
            parent: Some( Rc::downgrade(&parent) ),
        }));

        //create child childs ;)
        fill_tree(&mut root, depth-1);
        
        parent.borrow_mut().childs.push( root.clone() )
    }
}

fn show_parents(node: &Rc<RefCell<MyNode>>, out: &mut String) {
    if let Some(parent) = &node.borrow().parent {
        if let Some(parent) = parent.upgrade() {
            *out += &(parent.borrow().name.clone() + "--");
            show_parents(&parent, out);
        }
    }
    else {
        *out += "|";
    }
}

fn show_tree(tree: &Rc<RefCell<MyNode>>, depth: i32) {
    let mut indent: String = "".to_string();

    for i in 0..depth {
        indent += &" |-".to_string();
    }

    for i in tree.borrow().childs.iter() {
        let mut parents = String::from("");

        show_parents(i, &mut parents);

        println!("{}{}    parents={}", indent, i.borrow().name, parents);
        show_tree(i, depth+1); 
    }
}




fn main() {
    let mut root: Rc<RefCell<MyNode>>  = Rc::new( RefCell::new( MyNode {
        name: "rootnode".to_string(),
        childs: vec![],
        parent: None,
    }));

    fill_tree(&root, 3);
    show_tree(&root, 0);


}
