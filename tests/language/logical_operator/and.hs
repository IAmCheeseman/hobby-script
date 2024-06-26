// Note: These tests implicitly depend on ints being truthy

// Return the first non-true argument
print(false && 1); // expect: false
print(true && 1); // expect: 1
print(1 && 2 && false); // expect: false

// Return the last argument if all are true
print(1 && true); // expect: true
print(1 && 2 && 3); // expect: 3

// Short-circuit at the first false argument
var a = "before";
var b = "before";
(a = true) && (b = false) && (a = "bad");
print(a); // expect: true
print(b); // expect: false
