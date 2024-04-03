# Device Tree Library

To use the contents of a Device Tree, refer
to [this](https://github.com/devicetree-org/devicetree-specification/releases/download/v0.4/devicetree-specification-v0.4.pdf).

## Initialisation

At startup, the device tree is loaded by the bootloader in memory.
All we get is a `void*` giving us the address at which the Device Tree
is stored in memory. One can parse such a device tree with :

```c++
const DeviceTree dt(my_ptr);
```

Checks are performed at the instanciation of a `DeviceTree` object, so
you must check the object status before using it. One can perform such
check with :

```c++
if (dt.is_status_okay()) {
    // Everything went smoothly !
    // You're good to go
} else {
    // An error occurred parsing the DeviceTree header...
    // You cannot use :/
}
 ```

## The `DeviceTree` Object

A `DeviceTree` can be used to :

- Retrieve the DeviceTree version with `dt.get_version()`.
- Iterate over the reserved memory section (see section 5.3) with :

```c++
for (const MemorySection& section : dt.get_reserved_sections()) {
  std::cout << "Reserved section of size " << section.size << " starting at address " << section.address << std::endl;
}
```

- Retrieve the root of the Device Tree with `dt.get_root()`.
- Retrieve a node of the Device Tree with :

```c++
Node node_var = {};
if (dt.find_node("/my/node/path", &node_var)) {
  // Found the node, node_var is updated
} else {
  // No node found :/
}
```

- Retrieve a property of the Device Tree with :

```c++
Property prop_var = {};
if (dt.find_property("/my/property/path", &prop_var)) {
  // Found the property, prop_var is updated
} else {
  // No property found :/
}
```

Used path are those specified in the DeviceTree Specification.

## The `Node` Object

A node represent a set of property nested in a DeviceTree.
The following operation are possible :

- Retrieve the node's name with the `get_name()` function.
- Iterate over the children of a node with :

```c++
// Let 'node' be the node whose children you want to list.
for (const Node& child : node.get_children()) {
    // Do what you want to do with 'child' now.
}
```

- Iterate over the properties of a node with :

```c++
// Let 'node' be the node whose properties you want to list.
for (const Property& prop : node.get_properties()) {
  // Do what you want to do with 'prop' now.
}
```

- Find a child of a node, identified by its name with :

```c++
// Let 'node' be the node from which you want to select one of its children.
Node child = {};
if (node.find_child("my-child-name", &child)) {
  // Found the child, child is updated
} else {
  // No child found :/
}
```

- Find a property of a node, identified by its name with :

```c++
// Let 'node' be the node from which you want to select one of its properties.
Property prop_var = {};
if (node.find_property("my-property-name", &prop_var)) {
    // Found the property, prop_var is updated
} else {
    // No property found :/
}
```

## The `Property` Object

A Property is just a fancy wrapper for an array of data, its length, and a name.
For instance, if `prop` is a `Property` whose data is a C-string, you can display
it with :

```c++
std::cout << "Property '" << prop.name << "' (size: " << prop.length << ") : " << prop.data << std::endl;
```

Beware ! If the `data` field contains multi-bytes integers (or an array of), they will be encoded in big-endian.
