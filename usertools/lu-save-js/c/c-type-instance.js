import CInstance from "./c-instance.js";

export default class CTypeInstance extends CInstance {
   constructor(decl, type) {
      super(decl, type || decl?.c_types.serialized.definition);
   }
};