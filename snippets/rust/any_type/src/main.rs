use std::any::Any;

trait Widget {
    fn min_space(&self);
}

//-------------------------------------------
struct Checkbox {

}

impl Checkbox {
    fn new() -> Checkbox {
        return Checkbox{};
    }
}

impl Widget for Checkbox {
    fn min_space(&self) {

    }
}

//-------------------------------------------
struct List {

}

impl List {
    fn new() -> Checkbox {
        return Checkbox{};
    }
}

impl Widget for List {
    fn min_space(&self) {
        
    }
}
//-------------------------------------------



//-------------------------------------------
fn main() {
    //with String type
    {
        let s = "blabla".to_string();
        let s_ref: &dyn Any = &s;
    
        doit(s_ref);
    }

    //with own type
    {
        let checkbox = Checkbox::new();
        let checkbox_ref: &dyn Any = &checkbox;
        doit(checkbox_ref);
    }
  

    println!("Hello, world!");
}

fn doit(x: &dyn Any) {
    if let Some(x) = x.downcast_ref::<String>() {
        println!("{}", x);
    }
    else if let Some(x) = x.downcast_ref::<Checkbox>() {
        println!("it is a checkbox");
    }
    //doesnt work on trait
    //
    //else if let Some(x) = x.downcast_ref::<Checkbox>() {
    //    println!("it is a checkbox");
    //}
    else {
        print!("cannot make &dyn Any to String")
    }
}