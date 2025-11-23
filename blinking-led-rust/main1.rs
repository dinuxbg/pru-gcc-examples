#![feature(rustc_attrs)]

#[rustc_builtin_macro]
macro_rules! asm {
    () => {}
}

extern "C" {
    fn delay_us(input: u32) -> ();
}

fn main() {
    let ones: u32 = 0xffff;
    let zeros: u32 = 0;
    let period_us: u32 = 250 * 1000;
    loop {
        unsafe {
            asm!(
                "mov r30, {}",
                "nop",
                in(reg) ones,
            );
            delay_us(period_us);
            asm!(
                "mov r30, {}",
                "nop",
                in(reg) zeros,
            );
            delay_us(period_us);
        }
    }
}
