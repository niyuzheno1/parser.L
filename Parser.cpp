import * as Parser from "web-tree-sitter";
import * as fs from "fs";
const webAssemblyFile = "C:\\Users\\niyou\\Documents\\tree-sitter-cpp\\tree-sitter-cpp.wasm";
class NScope {
    private _scopeName = "";
    public get scopeName() {
        return this._scopeName;
    }
    public set scopeName(value) {
        this._scopeName = value;
    }
    private _allTemplateArguments: string[] = [];
    public get allTemplateArguments(): string[] {
        return this._allTemplateArguments;
    }
    public set allTemplateArguments(value: string[]) {
        this._allTemplateArguments = value;
    }
}



class NType {
    private _parentType: (NType | null) = null;
    public get parentType(): (NType | null) {
        return this._parentType;
    }
    public set parentType(value: (NType | null)) {
        this._parentType = value;
    }

    private _numPointers = 0;
    public get numPointers() {
        return this._numPointers;
    }
    public set numPointers(value) {
        this._numPointers = value;
    }
    private _arrayNumbers : string[] = [];
    public get arrayNumbers() {
        return this._arrayNumbers;
    }
    public set arrayNumbers(value) {
        this._arrayNumbers = value;
    }
    private _isOptional = false;
    public get isOptional() {
        return this._isOptional;
    }
    public set isOptional(value) {
        this._isOptional = value;
    }
    private _referencedCount = 0;
    public get referencedCount() {
        return this._referencedCount;
    }
    public set referencedCount(value) {
        this._referencedCount = value;
    }
    private _basetype: (NScope | string) = ""; // string means it is of primitive type
    public get basetype(): (NScope | string) {
        return this._basetype;
    }
    public set basetype(value: (NScope | string)) {
        this._basetype = value;
    }

    private _templateArgumentList: Array<string | NType | NScope> = [];
    public get templateArgumentList(): Array<string | NType | NScope> {
        return this._templateArgumentList;
    }
    public set templateArgumentList(value: Array<string | NType | NScope>) {
        this._templateArgumentList = value;
    }
    public cloneExceptPrimitives(oldval: NType){
        this._basetype = oldval.basetype;
        this._templateArgumentList = [];
        for (let i = 0; i < oldval.templateArgumentList.length; i++) {
            let x = oldval.templateArgumentList[i];
            this._templateArgumentList.push(x);
        }
    }
    

    public clone(oldval: NType) {
        this._basetype = oldval.basetype;
        this._templateArgumentList = [];
        for (let i = 0; i < oldval.templateArgumentList.length; i++) {
            let x = oldval.templateArgumentList[i];
            this._templateArgumentList.push(x);
        }
        this._numPointers = oldval.numPointers;
        //copy array numbers
        this._arrayNumbers = [];
        for (let i = 0; i < oldval.arrayNumbers.length; i++) {
            let x = oldval.arrayNumbers[i];
            this._arrayNumbers.push(x);
        }
        this._isOptional = oldval.isOptional;
        this._referencedCount = oldval._referencedCount;
        
    }
    //reset all the fields
    public reset() {
        this._basetype = "";
        this._templateArgumentList = new Array<string | NType | NScope>();
        this._numPointers = 0;
        this._arrayNumbers = [];
        this._isOptional = false;
        this._referencedCount = 0;
    }

    public tostring(){
        return this.toString();
    }
    
    public toString() {
        let ret = "";
        if(this.parentType !== null){
            ret += this.parentType.toString();
            ret += "::";
        }
        if (this.basetype instanceof NScope) {
            ret += this.basetype.scopeName;
        } else {
            ret += this.basetype;
        }
        if (this.templateArgumentList.length > 0) {
            ret += "<";
            for (let i = 0; i < this.templateArgumentList.length; i++) {
                
                if(true) {
                    const ourArgument = this.templateArgumentList[i];
                    if (ourArgument instanceof NType) {
                        ret += ourArgument.toString();
                    }
                    else if(ourArgument instanceof NScope) {
                        ret += ourArgument.scopeName;
                    }
                    else {
                        ret += ourArgument;
                    }
                    
                }
                if(i !== this.templateArgumentList.length - 1) {
                    ret += ",";
                }

            }
            
            ret += ">";
        }
        if (this.numPointers > 0) {
            ret += "*".repeat(this.numPointers);
        }
        if (this.referencedCount > 0) {
            ret += "&".repeat(this.numPointers);
        }
        if (this.arrayNumbers.length > 0) {
            for(const x of this.arrayNumbers) {
                if(x !== "NONE"){
                    ret += "[" + x + "]";
                }else{
                    ret += "[]";
                }
            }
        }
        if (this.isOptional) {
            ret += "?";
        }
        
        return ret;
    }
}

class NVariable {
    private _initializeString = "";
    public get initializeString() {
        return this._initializeString;
    }
    public set initializeString(value) {
        this._initializeString = value;
    }
    private _name: string = "";
    private _isStatic: boolean = false;
    public get isStatic(): boolean {
        return this._isStatic;
    }
    public set isStatic(value: boolean) {
        this._isStatic = value;
    }

    public toString(){
        let type = this.type.toString();
        let ret = "";
        if(type.indexOf("[") !== -1){
            let arrayCount = type.substring(type.indexOf("[") );
            type = type.substring(0, type.indexOf("["));
            ret = type.toString() + " " + this.name + arrayCount;
        }else{
            ret = this.type.toString() + " " + this.name ;
        }
        if(this.initializeString !== ""){
            ret = ret + "=" + this.initializeString;
        }
        return ret;
    }

    public get name(): string {
        return this._name;
    }
    public set name(value: string) {
        this._name = value;
    }
    private _type: NType = new NType();
    public get type(): NType {
        return this._type;
    }
    public set type(value: NType) {
        this._type = value;
    }
}

class VariableBuilder {
   
    setLastArraySize(arg0: string) {
        let last = this.variableList[this.variableList.length - 1];
        if(last.type.arrayNumbers.length === 0){
            return;
        }
        else{
            last.type.arrayNumbers[last.type.arrayNumbers.length - 1] = arg0;
        }

    }
    variableList : NVariable[] = [];
    currentType : NType = new NType();
    empty() : boolean {
        return this.variableList.length === 0;
    }
    addVariable(name: string, type: (NType| null) ) {
        let ntype = new NType();
        if(type){
            ntype.clone(type);
        }else{
            ntype.clone(this.currentType);
        }
        let nvar = new NVariable();
        nvar.name = name;
        nvar.type = ntype;
        this.variableList.push(nvar);
    }
    addType(type : NType){
        if(this.empty()) {return;}
        this.variableList[this.variableList.length - 1].type.clone(type);
    }
    addInitialization(initialized: string) {
        if(this.empty()) {return;}
        this.variableList[this.variableList.length - 1].initializeString = initialized;
    }
    addStatic(isStatic : boolean){
        if(this.empty()) {return;}
        this.variableList[this.variableList.length - 1].isStatic = isStatic;
    }
    addArraySize(size : string){
        if(this.empty()) {return;}
        this.variableList[this.variableList.length - 1].type.arrayNumbers.push(size);
    }
    addPointer(){
        if(this.empty()) {return;}
        this.variableList[this.variableList.length - 1].type.numPointers++;
    }
    addReference(){
        if(this.empty()) {return;}
        this.variableList[this.variableList.length - 1].type.referencedCount++;
    }
    setAllType(type : NType){
        for(let i = 0; i < this.variableList.length; i++){
            this.variableList[i].type.cloneExceptPrimitives(type);
        }
        this.currentType = type;
    }
    build() : NVariable[] {
        return this.variableList;
    }
}


class SourceControl {
    
}

class State {
    indent : number = 0;
}

class UpDates{
    currentType : NType = new NType();
    //identifier if we want to find numerals and functions for the sake of template
    requireType : boolean = false;
    variableBuilder : VariableBuilder = new VariableBuilder();
}

export function scanTopLevel(node : Parser.SyntaxNode) {
	return scanClosure(node, node.type, new State(), new UpDates());
}
function callDefault() {
    console.log("default");
}



export function scanClosure(node: Parser.SyntaxNode, category : string, state : State, update : UpDates) : string[]{
	let transcompilationResult : string[] = [];
	switch(category){
        
        
        case "dependent_type":
            if(true){
                for(const childNode of node.children){
                    switch(childNode.type){
                        default:
                            transcompilationResult = transcompilationResult.concat(scanClosure(childNode, childNode.type, state, update));
                            break;
                    }
                }
                let textBody = "";
                for(const x of transcompilationResult){
                    textBody += x;
                }
                transcompilationResult = [textBody];
            }
            break;
        
        
        case "qualified_identifier":
            if(true){
                let pretype : NType|null = null;
                for(const childNode of node.children){
                    switch(childNode.type){
                        case "primitive_type":
                        case "sized_type_specifier":
                        case "type_identifier":
                        case "template_type":
                            if(true){
                                let newUpdate = new UpDates();
                                newUpdate.requireType = true;
                                transcompilationResult = transcompilationResult.concat(scanClosure(childNode, childNode.type, state, newUpdate));
                                newUpdate.currentType.parentType = pretype;
                                pretype = newUpdate.currentType;
                                update.currentType = pretype;
                            }
                            break;
                        default:
                            //console.log(childNode.text, "+++", childNode.type);
                            transcompilationResult = transcompilationResult.concat(scanClosure(childNode, childNode.type, state, update));
                            break;
                    }
                }
                let textBody = transcompilationResult.join("");
                transcompilationResult = [textBody];
            }
            break;
        
        case "template_parameter_list":
            for(const childNode of node.children){
                switch(childNode.type){
                    default:
                        transcompilationResult = transcompilationResult.concat(scanClosure(childNode, childNode.type, state, update));
                        break;
                }
            }
            
            break;
        case "template_declaration":
            if(true){
                let headText = "";
                for(const childNode of node.children){
                    switch(childNode.type){
                        case "template":
                            headText += "template ";
                            break;
                        case "template_parameter_list":
                            headText += scanClosure(childNode, childNode.type, state, update).join("");
                            break;
                        default:
                            transcompilationResult = transcompilationResult.concat(scanClosure(childNode, childNode.type, state, update));
                            break;
                    }
                }
                transcompilationResult = [headText, ...transcompilationResult];
            }
            break;
        case "delete":
        case "new":
        case "typename":
        case "return":
        case "typedef":
        case "else":
        case "#ifdef":
        case "#ifndef":
        case "#else":
        case "#endif":
        case "using":
        case "namespace":
        case "lambda_capture_specifier":
        case "type_qualifier":
        case "#include":
        case "#define":
        case "private":
        case "protected":
        case "public":
        case "friend":
        case "explicit_function_specifier":
        case "storage_class_specifier":
            transcompilationResult = [node.text + " "];
            break;
        
        case "type_definition":
            if(true){
                for(const childNode of node.children){
                    switch(childNode.type){
                        default:
                            transcompilationResult = transcompilationResult.concat(scanClosure(childNode, childNode.type, state, update));
                            break;
                    }
                }
                let textBody = transcompilationResult.join("");
                transcompilationResult = [textBody];
            }
            break;
        
        case "if_statement":
        case "while_statement":      
            if(true){
                let headText = "";
                let firstparenthesis = false;
                for(const childNode of node.children){
                    switch(childNode.type){
                        case "compound_statement":
                            transcompilationResult = [...transcompilationResult , ...scanClosure(childNode, childNode.type, state, update)];
                            break;
                        case "condition_clause":
                            headText += scanClosure(childNode, childNode.type, state, update).join("");
                            firstparenthesis = true;
                            break;
                        default:
                            if(!firstparenthesis){
                                headText += scanClosure(childNode, childNode.type, state, update).join("");
                            }else{
                                let copyState = {...state};
                                copyState.indent ++;
                                let y = scanClosure(childNode, childNode.type, copyState, update);
                                if(childNode.type === "else"){
                                    copyState.indent--;
                                }
                                for(let i = 0; i < y.length; i++){
                                    y[i] = "\t".repeat((copyState.indent)) + y[i];
                                }
                                transcompilationResult = [...transcompilationResult, ...y];
                            }
                            break;
                    }
                } 
                transcompilationResult = [ headText, ...transcompilationResult];
                let bodyText = transcompilationResult.join("\n");
                transcompilationResult = [bodyText];
            }
            break;
        
            
        case "template_argument_list":
            for(const childNode of node.children){
                switch(childNode.type){
                    case "number_literal":
                    case "type_descriptor":
                        if(true){
                            let newUpdate = new UpDates();
                            newUpdate.requireType = true;
                            transcompilationResult = transcompilationResult.concat(scanClosure(childNode, childNode.type, state, newUpdate));
                            update.currentType.templateArgumentList.push(newUpdate.currentType);
                        }
                        break;
                    default:
                        transcompilationResult = transcompilationResult.concat(scanClosure(childNode, childNode.type, state, update));
                        break;
                    }
            }
            
            break;
        case "template_function":
            if(true){
                for(const childNode of node.children){
                    switch(childNode.type){
                        default:
                            transcompilationResult = transcompilationResult.concat(scanClosure(childNode, childNode.type, state, update));
                            break;
                        }
                }
                let textBody = transcompilationResult.join("");
                transcompilationResult = [textBody];    
            }
            break;
        case "for_range_loop":
        case "for_statement":
            if(true){
                let headText = "";
                let firstparenthesis = false;
                for(const childNode of node.children){
                    switch(childNode.type){
                        case "compound_statement":
                            transcompilationResult = [...transcompilationResult , ...scanClosure(childNode, childNode.type, state, update)];
                            break;
                        case ")":
                            headText += scanClosure(childNode, childNode.type, state, update).join("");
                            firstparenthesis = true;
                            break;
                        default:
                            if(!firstparenthesis){
                                headText += scanClosure(childNode, childNode.type, state, update).join("");
                            }else{
                                let copyState = {...state};
                                copyState.indent ++;
                                let y = scanClosure(childNode, childNode.type, copyState, update);
                                for(let i = 0; i < y.length; i++){
                                    y[i] = "\t".repeat((copyState.indent)) + y[i];
                                }
                                transcompilationResult = [...transcompilationResult, ...y];
                            }
                            break;
                    }
                } 
                
                transcompilationResult = [ headText, ...transcompilationResult];
                let bodyText = transcompilationResult.join("\n");
                transcompilationResult = [bodyText];
            }
            break;
        
        case "type_descriptor":
            for(const childNode of node.children){
                switch(childNode.type){
                    default:
                        transcompilationResult = [...transcompilationResult , ...scanClosure(childNode, childNode.type, state, update)];
                        break;
                }
            } 
            break;
        case "field_expression":
            if(true){
                for(const childNode of node.children){
                    switch(childNode.type){
                        default:
                            transcompilationResult = [...transcompilationResult , ...scanClosure(childNode, childNode.type, state, update)];
                            break;
                    }
                }    
                let joinchar = "";
                let textbody = transcompilationResult.join(joinchar); 
                let newline = 0;
                transcompilationResult = [textbody];
            }
            break;
        case "...":
            transcompilationResult = [" " + node.text];
            break;
        case "decltype":
            if("decltype"  === node.text){
                transcompilationResult = ["decltype"];
                break;
            }
        case "break_statement":
        case "continue_statement":
        case "condition_clause":
        case "base_class_clause":
        case "parameter_pack_expansion":
        case "operator_name":
        case "operator_cast":
        case "field_initializer":
        case "field_initializer_list":
        case "assignment_expression":
        case "sizeof_expression":
        case "pointer_expression":
        case "parenthesized_expression":
        case "cast_expression":
        case "unary_expression":
        case "return_statement":
        case "delete_expression":
        case "new_expression":
        case "lambda_expression":
        case "subscript_expression":
        case "update_expression":
        case "binary_expression":
        case "comma_expression":
        case "conditional_expression":
        case "variadic_declarator":
        case "new_declarator":
        case "type_parameter_declaration":
        case "variadic_parameter_declaration":
        case "variadic_type_parameter_declaration":
        case "static_assert_declaration":
        case "alias_declaration":
        case "optional_type_parameter_declaration":
        case "friend_declaration":
        case "abstract_function_declarator":
        case "trailing_return_type":
            if(true){
                for(const childNode of node.children){
                    switch(childNode.type){
                        default:
                            transcompilationResult = [...transcompilationResult , ...scanClosure(childNode, childNode.type, state, update)];
                            break;
                    }
                }    
                let joinchar = "";
                if(node.type === "type_parameter_declaration" 
                    || node.type === "optional_type_parameter_declaration"
                    || node.type === "operator_cast"){
                    joinchar = " ";
                }
                let textbody = transcompilationResult.join(joinchar); 
                let newline = 0;
                transcompilationResult = [textbody];
            }
            break;
        case "optional_parameter_declaration":
        case "field_declaration":
        case "parameter_declaration":
        case "declaration":
            if(true){
                let newUpdate = new UpDates();
                newUpdate.variableBuilder = new VariableBuilder();
                let equalSymbol = false;
                for(const childNode of node.children){
                    switch(childNode.type){
                        case "auto":
                        case "primitive_type":
                        case "sized_type_specifier":
                        case "type_identifier":
                        case "template_type":
                            if(true && !equalSymbol){
                                newUpdate.currentType = new NType();
                                transcompilationResult = [...transcompilationResult , ...scanClosure(childNode, childNode.type, state, newUpdate)];
                                newUpdate.variableBuilder.setAllType(newUpdate.currentType);    
                            }else{
                                transcompilationResult = [...transcompilationResult , ...scanClosure(childNode, childNode.type, state, newUpdate)];
                            }
                            break;
                        case "=":
                            equalSymbol = true;
                            transcompilationResult = [...transcompilationResult , ...scanClosure(childNode, childNode.type, state, newUpdate)];
                            break;
                        case "field_identifier":
                        case "identifier":
                            if(!equalSymbol){
                                newUpdate.variableBuilder.addVariable(childNode.text, null);
                                transcompilationResult = [...transcompilationResult , ...scanClosure(childNode, childNode.type, state, newUpdate)];    
                            }else{
                                transcompilationResult = [...transcompilationResult , ...scanClosure(childNode, childNode.type, state, newUpdate)];
                            }
                            break;
                        default:
                            transcompilationResult = [...transcompilationResult , ...scanClosure(childNode, childNode.type, state, newUpdate)];
                            break;
                    }
                }
                
                if(node.type === "optional_parameter_declaration"){
                    let ifEqual = false;
                    let initializer = "";
                    for(const x of transcompilationResult){
                        if(ifEqual){
                            initializer += x;
                        }
                        if(x === "="){
                            ifEqual = true;
                        }
                    }
                    newUpdate.variableBuilder.addInitialization(initializer);

                }
                let variables = newUpdate.variableBuilder.build(); 
                transcompilationResult = [];
                for(const variable of variables){
                    if(node.type === "declaration" || node.type === "field_declaration"){
                        transcompilationResult.push(variable.toString() + ";");
                    }else{
                        transcompilationResult.push(variable.toString());
                    }
                }   
                // let textbody = transcompilationResult.join(""); 
                let newline = 0;
                // transcompilationResult = [textbody];
            }
            break;
        case "call_expression":
            if(true){
                let headText = "";
                let replacableText = false;
                for(const childNode of node.children){
                    switch(childNode.type){
                        case "identifier":
                        case "field_identifier":
                            headText = childNode.text;
                            if(replacable(headText)){
                                replacableText = true;
                                continue;
                            }else{
                                transcompilationResult = [...transcompilationResult , ...scanClosure(childNode, childNode.type, state, update)];
                            }
                            break;
                        default:
                            transcompilationResult = [...transcompilationResult , ...scanClosure(childNode, childNode.type, state, update)];
                            break;
                    }
                }    
                let joinchar = "";
                let textbody = "";
                if(replacableText){
                    textbody = replacement(headText, transcompilationResult);
                }else{
                    textbody = transcompilationResult.join(joinchar); 
                }
                let newline = 0;
                transcompilationResult = [textbody];
            }
            break;
        
        case "expression_statement":
            for(const childNode of node.children){
                switch(childNode.type){
                    case ";":
                        transcompilationResult[transcompilationResult.length-1] += ";";
                        break;
                    default:
                        transcompilationResult = [...transcompilationResult , ...scanClosure(childNode, childNode.type, state, update)];
                        break;
                }
            }    
            break;
        case "compound_statement":
            if(true){
                let copyState = {...state};
                copyState.indent++;
                for(const childNode of node.children){
                    switch(childNode.type){
                        default:
                            transcompilationResult = [...transcompilationResult , ...scanClosure(childNode, childNode.type, copyState, update)];
                            break;
                    }
                }    
                for(let i = 0; i < transcompilationResult.length; i++){
                    let x = transcompilationResult[i];
                    if(x === "{" || x === "}") {
                        transcompilationResult[i] = "\t".repeat((state.indent)) + x;
                    }else{
                        transcompilationResult[i] = "\t".repeat((state.indent+1)) + x;
                    }

                }
                let bodyText = transcompilationResult.join("\n");
                transcompilationResult = [bodyText];
            }
            break;
        
        
        case "namespace_identifier":
        case "number_literal":
            if(update.requireType){
                update.currentType = new NType();
                update.currentType.basetype = node.text;
            }
        case "identifier":
        case "field_identifier":
            transcompilationResult.push(node.text);
            break;
        case "auto":
        case "primitive_type":
        case "sized_type_specifier":
        case "type_identifier":
            if(true){
                let attemptedReplacement = typeReplacement(node.text);
                if(attemptedReplacement !== node.text){
                    //we will do something here to make sure the type we return from update is correct
                }else{
                    update.currentType = new NType();
                    update.currentType.basetype = node.text;
                }
                transcompilationResult.push(attemptedReplacement + " ");
            }
            break;
        case "template_type":
            if(true){
                for(const childNode of node.children){
                    switch(childNode.type){
                        case "primitive_type":
                        case "sized_type_specifier":
                        case "type_identifier":
                            if(true){
                                let newUpdate = new UpDates();
                                transcompilationResult = [...transcompilationResult , ...scanClosure(childNode, childNode.type, state, newUpdate)];
                                update.currentType = newUpdate.currentType;
                            }
                            break;
                        case "template_argument_list":
                            if(true){
                                let newUpdate = new UpDates();
                                //copy update.currentType into newUpdate.currentType
                                newUpdate.currentType.clone(update.currentType);
                                transcompilationResult = [...transcompilationResult , ...scanClosure(childNode, childNode.type, state, newUpdate)];
                                update.currentType = newUpdate.currentType;
                            }
                            break;
                        default:
                            transcompilationResult = [...transcompilationResult , ...scanClosure(childNode, childNode.type, state, update)];
                            break;
                    }
                }    
                let resultent = update.currentType.toString() + " ";//transcompilationResult.join("");
                transcompilationResult = [resultent];
            }
            break;
        case "argument_list":
        case "parameter_list":
            if(true){
                for(const childNode of node.children){
                    switch(childNode.type){
                        default:
                            transcompilationResult = [...transcompilationResult , ...scanClosure(childNode, childNode.type, state, update)];
                            break;
                    }
                }    
                let resultent = transcompilationResult.join("");
                transcompilationResult = [resultent];
            }
            break;
        case "init_declarator":
            console.log("init_declarator");
        case "array_declarator":
        case "pointer_declarator":
        case "reference_declarator":
            if(true){
                let type = "";
                let pointercounts = 0;
                let referencecounts = 0;
                let name = "";
                let bracketEcho = 0; //[]
                let equalSymbol = false;
                for(const childNode of node.children){
                    switch(childNode.type){
                        case "*":
                            if(!equalSymbol){
                                pointercounts++;
                                transcompilationResult = [...transcompilationResult , ...scanClosure(childNode, childNode.type, state, update)];
                            }else{
                                transcompilationResult = [...transcompilationResult , ...scanClosure(childNode, childNode.type, state, update)];
                            }
                            break;
                        case "&":
                            if(!equalSymbol){
                                referencecounts++;
                                transcompilationResult = [...transcompilationResult , ...scanClosure(childNode, childNode.type, state, update)];
                            }else{
                                transcompilationResult = [...transcompilationResult , ...scanClosure(childNode, childNode.type, state, update)];
                            }
                            break;
                        case "field_identifier":
                        case "identifier":
                            if(bracketEcho === 0 && !equalSymbol){
                                name = childNode.text;
                                transcompilationResult = [...transcompilationResult , ...scanClosure(childNode, childNode.type, state, update)];
                                update.variableBuilder.addVariable(name, null);    
                            }else{
                                let partialTranslated = scanClosure(childNode, childNode.type, state, update);
                                if(!equalSymbol){
                                    update.variableBuilder.setLastArraySize(partialTranslated[0]);
                                }
                                transcompilationResult = [...transcompilationResult , ...partialTranslated];
                            }
                            break;
                        case "[":
                            if(!equalSymbol){
                                update.variableBuilder.addArraySize("NONE");
                                ++bracketEcho;
                            }else{
                                transcompilationResult = [...transcompilationResult , ...scanClosure(childNode, childNode.type, state, update)];
                            }
                            break;
                        case "]":
                            if(!equalSymbol){
                                --bracketEcho;
                            }else{
                                transcompilationResult = [...transcompilationResult , ...scanClosure(childNode, childNode.type, state, update)];
                            }
                            break;
                        case "=":
                            equalSymbol = true;
                        default:
                            if(bracketEcho > 0 && !equalSymbol){
                                let partialTranslated = scanClosure(childNode, childNode.type, state, update);
                                update.variableBuilder.setLastArraySize(partialTranslated[0]);
                            }else{
                                transcompilationResult = [...transcompilationResult , ...scanClosure(childNode, childNode.type, state, update)];
                            }
                            break;
                    }
                }  
                //build variable
                
                for(let i = 0 ; i < pointercounts; i++){
                    update.variableBuilder.addPointer();
                }
                for(let i = 0 ; i < referencecounts; i++){
                    update.variableBuilder.addReference();
                }

                let initialized = "";
                let ifEqual = false;
                for(let i = 0; i < transcompilationResult.length; ++i){
                    if(ifEqual){
                        initialized += transcompilationResult[i];
                    }
                    if(transcompilationResult[i] === "="){
                        ifEqual = true;
                    }
                }
                if(ifEqual){
                    update.variableBuilder.addInitialization(initialized);
                }

                let textbody = transcompilationResult.join("");
                transcompilationResult = [textbody];
            }
            break;
        

        
        case "function_declarator":
            if(true){
                for(const childNode of node.children){
                    switch(childNode.type){
                        default:
                            transcompilationResult = [...transcompilationResult , ...scanClosure(childNode, childNode.type, state, update)];
                            break;
                    }
                } 
                let bodyText = transcompilationResult.join("");
                transcompilationResult = [bodyText];
            }
            break;
        case "function_definition":
            if(true){
                for(const childNode of node.children){
                    switch(childNode.type){
                        case "compound_statement":
                            transcompilationResult.push("\n");
                            transcompilationResult = [...transcompilationResult , ...scanClosure(childNode, childNode.type, state, update)];
                            break;
                        default:
                            transcompilationResult = [...transcompilationResult , ...scanClosure(childNode, childNode.type, state, update)];
                            break;
                    }
                } 
                let bodyText = transcompilationResult.join("");
                transcompilationResult = [bodyText];
            }
            break;
        case "declaration_list":
        case "field_declaration_list":
            if(true){
                let copyState = {...state};
                copyState.indent++;
                for(const childNode of node.children){
                    switch(childNode.type){
                        default:
                            transcompilationResult = [...transcompilationResult , ...scanClosure(childNode, childNode.type, copyState, update)];
                            break;
                    }
                } 
                for(let i = 0; i < transcompilationResult.length; i++){
                    let x = transcompilationResult[i];
                    if(x === "{" || x === "}") {
                        transcompilationResult[i] = "\t".repeat((state.indent)) + x;
                        continue;   
                    }
                    transcompilationResult[i] = "\t".repeat((copyState.indent)) + x;
                }
                let bodyText = transcompilationResult.join("\n");
                transcompilationResult = [bodyText];
            }
            break;
        case "namespace_definition":
        case "struct_specifier":
        case "class_specifier":
            if(true){
                for(const childNode of node.children){
                    switch(childNode.type){
                        default:
                            transcompilationResult = [...transcompilationResult , ...scanClosure(childNode, childNode.type, state, update)];
                            break;
                    }
                }
                transcompilationResult.push(";"); 
                let bodyText = transcompilationResult.join(" ");
                transcompilationResult = [bodyText];
            }
            break;
        case "abstract_pointer_declarator":
        case "abstract_reference_declarator":
        case "noexcept":
        case "static_assert":
        case "?":
        case "~":
        case "char_literal":
        case "destructor_name":
        case "preproc_directive":
        case "system_lib_string":
        case "[]":
        case "initializer_list":
        case "nullptr":
        case "null":
        case "string_literal":
        case "-":
        case "*":
        case "/":
        case "%":
        case "^":
        case "&":
        case "|":
        case "&&":
        case "||":
        case "==":
        case "!=":
        case "+":
        case ":":
        case "::":
        case ">=":
        case "<=":
        case "!":
        case "--":
        case ">":
        case "+=":
        case "-=":
        case "*=":
        case "/=":
        case "%=":
        case "^=":
        case "&=":
        case "|=":
        case "<<=":
        case ">>=":
        case "&&=":
        case "||=":
        case "false":
        case "true":
        case ",":
        case "for":
        case "[":
        case "]":
        case "{":
        case "}":
        case "++":
        case "+=":
        case ".":
        case "(":
        case ")":
        case "<":
        case "<<":
        case ">>":
        case "if":
        case "while":
        case "do":
        case "break":
        case "continue":
        case "goto":
        case "switch":
        case "case":
        case "default":
        case "struct":
        case "union":
        case "enum":
        case "sizeof":
        case "()":
        case "operator":
        case ";" :  
        case "this":
        case "->":
        case "=":
        case "class":
        case "access_specifier":
        case "ref_qualifier":
        case "preproc_arg":
            transcompilationResult = [node.text];
            break;
        
        case "preproc_call":
            if(true){
                for(const childNode of node.children){
                    switch(childNode.type){
                        default:
                            transcompilationResult = [...transcompilationResult , ...scanClosure(childNode, childNode.type, state, update)];
                            break;
                    }
                } 
                let bodyText = transcompilationResult.join(" ");
                transcompilationResult = [bodyText];
            }
            break;
        
        case "comment":
            //not implemented yet 
            break;
       
        case "preproc_def":
            if(true){
                for(const childNode of node.children){
                    switch(childNode.type){
                        default:
                            transcompilationResult = [...transcompilationResult , ...scanClosure(childNode, childNode.type, state, update)];
                            break;
                    }
                } 
                let bodyText = transcompilationResult.join(" ");
                transcompilationResult = [bodyText];
            }
                    
            
            break;
        case "preproc_ifdef":
            for(const childNode of node.children){
                switch(childNode.type){
                    default:
                        if(transcompilationResult.length === 0 || transcompilationResult[transcompilationResult.length - 1] !== "#ifdef "){
                            transcompilationResult = [...transcompilationResult , ...scanClosure(childNode, childNode.type, state, update)];
                        }else{
                            transcompilationResult[transcompilationResult.length - 1] += scanClosure(childNode, childNode.type, state, update)[0];
                        }
                        break;
                }
            }          
            break;        
        
        case "translation_unit":
            for(const childNode of node.namedChildren){
                switch(childNode.type){
                    default:
                        transcompilationResult = [...transcompilationResult , ...scanClosure(childNode, childNode.type, state, update)];
                        break;
                }
            }                    
            break;            
        
        case "using_declaration":
        case "preproc_include":
            if(true){
                for(const childNode of node.children){
                    switch(childNode.type){
                        default:
                            transcompilationResult = [...transcompilationResult , ...scanClosure(childNode, childNode.type, state, update)];
                            break;
                    }
                }
                let bodyText = transcompilationResult.join("");
                transcompilationResult = [bodyText];
            }
            break;
        case "\n":
            break;
        
		default:
            console.log(category, "+++" , node.text);
            break;
	}
	
	return transcompilationResult;
}

async function readParser(){
    await Parser.init();
    const parser = new Parser();
    const lang = await Parser.Language.load(webAssemblyFile);
    parser.setLanguage(lang);
    //read from test.cpp file
    const file = fs.readFileSync("C:\\Users\\niyou\\Documents\\parserTest\\test.cpp", "utf8");
    const tree = parser.parse(file);
    const rootNode = tree.rootNode;
    const aggregateResult_ = scanTopLevel(rootNode);
    const result = [];
    result.push("#pragma GCC optimize(\"Ofast\")");
    result.push("#pragma GCC optimize(\"unroll-loops\")");
    result.push("#pragma GCC optimize(\"inline\")");
    result.push("#include<bits/stdc++.h>");
    result.push("using namespace std;");
    result.push("");
    for(const line of aggregateResult_){
        if(line.length > 0){
            result.push(line);
        }
    }
    //write result to output.cpp file with all the lines
    fs.writeFileSync("C:\\Users\\niyou\\Documents\\parserTest\\output.cpp", result.join("\n"));
}

function main(){
    readParser();
}

main();
function replacement(headText: string, transcompilationResult: string[]) {
    let returnText = "";
    if(headText === "sz"){
        returnText = "(int)"+transcompilationResult[0] + ".size()"; 
    }
    if(headText === "all"){
        returnText = transcompilationResult[0] + ".begin(), " + transcompilationResult[0] + ".end()"; 
    }
    return returnText;
}

function replacable(headText: string) : boolean {
        if(headText === "sz"){
            return true;
        }
        if(headText === "all"){
            return true;
        }
        return false;
}



function typeReplacement(text: string) {
    switch(text){
        case "VI":
            return "vector<int>";
        case "VS":
            return "vector<string>";
        case "VII":
            return "vector<vector<int>>";
        case "ld":
            return "long double";
        case "ll":
            return "long long";
        case "cd":
            return "complex<long double>";
        case "pii":
            return "pair<int, int>";
        case "pll":
            return "pair<long long, long long>";
        case "pld":
            return "pair<long double, long double>";
        case "VD":
            return "vector<double>";
        case "VL":
            return "vector<long>";
        case "VPI":
            return "vector<pair<int, int>>";
        case "VPL":
            return "vector<pair<long long, long long>>";
        case "VCD":
            return "vector<complex<long double>>";
        default:
            break;
    }
    return text;
}

