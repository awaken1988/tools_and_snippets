

#[derive(Debug)]
//TODO: can we make implement template here?
pub struct MyVector {
    pub x: f64,
    pub y: f64,
    pub z: f64,

    //TODO: can we implement an index operator here?
}

impl MyVector {
    pub fn new() -> MyVector {
        MyVector {
            x: 0.0,
            y: 0.0,
            z: 0.0,
        }
    }

    pub fn sum(&self) -> f64 {
        return self.x + self.y + self.z;
    }
}