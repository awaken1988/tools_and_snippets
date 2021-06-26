use std::collections::HashMap;

pub mod defs;
mod xorshift64;



//TODO: try tempalte variant
fn append_gener(x: & mut HashMap<String,defs::GeneratorBuilder>, y: defs::GeneratorBuilder) {
    let name = y.name.clone();

    x.insert(name, y);
}

pub fn get_all() -> HashMap<String,defs::GeneratorBuilder> {
    let mut geners: HashMap<String,defs::GeneratorBuilder> = HashMap::<>::new();

    append_gener(&mut geners, xorshift64::get());

    return geners;
}