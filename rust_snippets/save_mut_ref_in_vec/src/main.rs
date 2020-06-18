
trait Widget {
    fn min_space(&self);
}

struct List {
   
}

impl List {
    fn new() -> List {
        return List{};
    }
}

impl Widget for List {
    fn min_space(&self) {

    }
}

struct LayoutImpl<'a> {
    widget: Vec<&'a mut dyn Widget>,
}

impl<'a> LayoutImpl<'a> {
    fn new() -> LayoutImpl<'a> {
        return LayoutImpl{ widget: vec![]};
    }

    fn add(&mut self, w: &'a mut dyn Widget) {
        self.widget.push(w);
    }

    fn doit(&mut self) {

    }
}


fn main() {
    let mut widget0 = List::new();
    let mut widget1 = List::new();
    let mut widget2 = List::new();

    let mut layout = LayoutImpl::new();

    layout.add(&mut widget0);

    

    layout.doit();
    widget0.min_space();

}
