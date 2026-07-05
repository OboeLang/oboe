just throwing other questions here.

was looking stuff up and

> Whether operators are overloadable/user-definable (and if so, can users define new operators, like Haskell, or just overload existing ones, like C++/Python).

i like both of these honestly?? dunno how hard the first would be to implement.

```
operator ||> {
    let c = a * 10
    return c + b
}
```
with a being the left side and b being the right side. so like

```
// after defining that operator

print(5 ||> 8) // prints 58
```

and for the second thing

```
class Vector2 {
    func init(this, float x, float y) {
        let this.x = x
        let this.y = y
    }

    func $add(this, var other) { // overrides +, all operators would only have the object that isn't this
        return Vector2(this.x + other.x, this.y + other.y)
    }
}
```


> Special operators for your domain (e.g., pipeline |>, null-coalescing ??, spread ...).

yeah i dont actuakly know what this one means

kinda

do we want to do either of those?


