var yang = require("./index")

const ctx = new yang.Context("./files", 0);
var module = ctx.parse_module_path("./files/b.yang", yang.LYS_IN_YANG);

console.log(module.name());
