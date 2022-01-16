import * as Parser from "web-tree-sitter";
import * as fs from "fs";
import { assert } from "console";
const webAssemblyFile = "C:\\Users\\niyou\\Documents\\tree-sitter-cpp\\tree-sitter-cpp.wasm";

const sourceControlSwitch = true; 
let parser : (Parser | null) = null;
// string getUnusedVarName(void){
function getUnusedVarName(node : Parser.SyntaxNode, state : State, update : UpDates) : string {
//     int i, k, p, r;
    let f = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_";
    let res = "";
    for(let k = 8; ; k++) {
        for(let p = 0; p < 100; p++) {
            res = "";
            let r = Math.floor(Math.random() * 52);
            res += f[r];
            for(let i = 1; i < k; i++) {
                //r = rand() % f.length();
                r = Math.floor(Math.random() * 63);
                res += f[r];
            }

//       if(localvar.count(res)) continue;
//       if(argvar.count(res)) continue;
//       if(globalvar.count(res)) continue; //not realized yet

          return res;
        }
    }


    return res;
}

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
    name : string = "";
    sourceCode : string[] = [];
    dependencies : Map<string, number> = new Map<string, number>();
    type : string = ""; //front or back or initializing
    declaredIdentifiers : string[] = [];
    priorities : number = 0;
    context : string[] = [];
}

interface SC {
    name : string;
    sourceCode : string[];
    dependencies : string[];
    type : string;
    identifiers : string[];
    priorities : number ;
    context : string[];
}

const blockSC : SourceControl[] = [];
const allDependencies : Map<string, number> = new Map<string, number>();
class State {
    indent : number = 0;
    fileName : string = "";
    scope : NScope = new NScope();
    sourceControlName : string = "";
    currentcontext : string[] = [];
    topLevel : boolean = false;
}

class UpDates{
    currentType : NType = new NType();
    //identifier if we want to find numerals and functions for the sake of template
    requireType : boolean = false;
    variableBuilder : VariableBuilder = new VariableBuilder();
    sourceControl : SourceControl = new SourceControl();
}

export function scanTopLevel(node : Parser.SyntaxNode, fileName : string) {
    let state = new State();
    state.fileName = fileName; 
	return scanClosure(node, node.type, state, new UpDates());
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
                                copyState.scope = state.scope;
                                copyState.indent ++;
                                let y : string[] = [];
                                //if(childNode.type.indexOf("expression") !== -1){
                                if(childNode.type !== "compound_statement" && childNode.type !== "else"){
                                    let newText = "{" + echoReplace(childNode, state, update, ",", ";") + "}";
                                    if(parser){
                                        copyState.indent--;
                                       let newTextNode = parser.parse(newText).rootNode;
                                        assert( newTextNode.children[0].type === "compound_statement");
                                        y = scanClosure(newTextNode.children[0], "compound_statement", copyState, update);
                                    }
                                }else{
                                     y = scanClosure(childNode, childNode.type, copyState, update);
                                     if(childNode.type === "else"){
                                        copyState.indent--;
                                    }
                                    for(let i = 0; i < y.length; i++){
                                        y[i] = "\t".repeat((copyState.indent)) + y[i];
                                    }
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
        
        case "repeat_statement":
            if(true){
                let headText = "";
                let structure = "";
                let hasbracket = false;
                let defaultSlots : string[] =  [];
                let code : string = "";
                let endOfParenthesis = false;
                for(const childNode of node.children){
                    switch(childNode.type){
                        case "[":
                            structure += childNode.text;
                            break;
                        case "]":
                            structure += childNode.text;
                            break;
                        case "(":
                            structure += childNode.text;
                            break;
                        case ")":
                            endOfParenthesis = true;
                            structure += childNode.text;
                            break;
                        case ",":
                            structure += childNode.text;
                            break;
                        case "compound_statement":
                            // let copyState = {...state};
                            // copyState.scope = state.scope;
                            
                            // hasbracket = true;
                            // if(headText === ""){
                            //     headText = childNode.text;
                            // }
                            // let y = scanClosure(childNode, childNode.type, copyState, update);
                            
                            // defaultSlots = [...defaultSlots, ...y];
                            code += childNode.text;
                            break;
                        default:
                            if(headText === ""){
                                headText = childNode.text;
                            }
                            if(!endOfParenthesis){
                                defaultSlots.push(scanClosure(childNode, childNode.type, state, update).join(" "));
                            }else{
                                code += "{";
                                code += childNode.text;
                                code += "}";
                            }
                            break;
                    }
                }
                if(headText.indexOf("_") === -1){
                    // int REP_fg = 0, rrep_fg = 0;
                    // eslint-disable-next-line @typescript-eslint/naming-convention
                    let REP_flag = false, rrep_flag = false;
                    if(headText.indexOf("REP") !== -1){
                        REP_flag = true;
                    }
                    if(headText.length >= 4){
                        rrep_flag = true;
                    }
                    let stringArgs = ["", "", "", ""];
                    let additionalClause = "";
                    let additionalHeader : string[] = [];  // for REP_fg
                    let cnt = 0;
                    for(let i = 0; i < structure.length; i++){
                        if(structure[i] === ",")
                        {
                            cnt++;
                        }
                    }
                    let tmpArgs : string[] = [];
                    if(structure.indexOf("[]") !== -1){
                        let indices = getUnusedVarName(node, state, update);
                        additionalClause = "auto & " + defaultSlots[2] + " = " + defaultSlots[1] + "[" + indices  + "];";
                        tmpArgs.push(indices);
                        for(let i = 3; i < defaultSlots.length; ++i){
                            if(3 <= i && i <= 3 + cnt - 1){
                                tmpArgs.push(defaultSlots[i]);
                            }else{
                                transcompilationResult.push(defaultSlots[i]);
                            }
                        }
                    }else{
                        for(let i = 1; i < defaultSlots.length ; ++i){
                            if(1 <= i && i <= 1+cnt){
                                tmpArgs.push(defaultSlots[i]);
                            }else{
                                transcompilationResult.push(defaultSlots[i]);
                            }
                        }    
                        if(cnt === 0){
                            let uVariable = getUnusedVarName(node, state, update);
                            tmpArgs = [uVariable, ...tmpArgs];
                        }
                        if(REP_flag){
                            switch(cnt){
                                case 0:
                                    if(true){
                                        let uVariable = getUnusedVarName(node, state, update);
                                        additionalHeader.push("int " + uVariable + " = (int)(" + tmpArgs[1] + ");");
                                        tmpArgs[1] = uVariable;
                                    }
                                    break;
                                case 1:
                                    if(true){
                                        let uVariable = getUnusedVarName(node, state, update);
                                        additionalHeader.push("int " + uVariable + " = (int)(" + tmpArgs[2] + ");");
                                        tmpArgs[2] = uVariable;
                                    }
                                    break;
                                default:
                                    if(true){
                                        let uVariable = getUnusedVarName(node, state, update);
                                        additionalHeader.push("int " + uVariable + " = (int)(" + tmpArgs[3] + ");");
                                        tmpArgs[3] = uVariable;
                                    }
                                    break;
                            }
                        }
                    }
                    switch(cnt){
                        case 0:
                            if(true){
                                stringArgs[0] = tmpArgs[0];
                                stringArgs[1] = "0";
                                stringArgs[2] = tmpArgs[1];
                                if(rrep_flag === true){
                                    stringArgs[3] = "++";
                                }else{
                                    stringArgs[3] = "--";
                                }    
                            }
                            break;
                        case 1:
                            if(true){
                                stringArgs[0] = tmpArgs[0];
                                stringArgs[1] = "0";
                                stringArgs[2] = tmpArgs[1];
                                if(rrep_flag === true){
                                    stringArgs[3] = "++";
                                }else{
                                    stringArgs[3] = "--";
                                }    
                            }
                            break;
                        case 2:
                            if(true){
                                stringArgs[0] = tmpArgs[0];
                                stringArgs[1] = tmpArgs[1];
                                stringArgs[2] = tmpArgs[2];
                                if(rrep_flag === true){
                                    stringArgs[3] = "++";
                                }else{
                                    stringArgs[3] = "--";
                                }  
                            }
                            break;
                        case 3:
                            if(true){
                                stringArgs[0] = tmpArgs[0];
                                stringArgs[1] = tmpArgs[1];
                                stringArgs[2] = tmpArgs[2];
                                if(rrep_flag === true){
                                    stringArgs[3] = "+=(" + tmpArgs[3]+")";
                                }else{
                                    stringArgs[3] = "-=(" + tmpArgs[3]+")";
                                }  
                            }
                            break;
                    }
                    if(rrep_flag){
                        let tmp = stringArgs[1];
                        stringArgs[1] = stringArgs[2];
                        stringArgs[2] = tmp;
                        stringArgs[1] = "(" + stringArgs[1] + ")-1";
                        stringArgs[2] = ">=(" +  stringArgs[2] + ")";
                    }else{
                        stringArgs[1] = "(" + stringArgs[1] + ")";
                        stringArgs[2] = "<(" + stringArgs[2] + ")";
                    }
                    additionalHeader.push("for(int " + stringArgs[0] + " = " + stringArgs[1] + "; "+ stringArgs[0]  + stringArgs[2] + "; " + stringArgs[0]  + stringArgs[3] + ")");
                    // if(!hasbracket){
                    if(true){
                        // let newText = ["{" , ...transcompilationResult, "}"];
                        if(parser){
                            //find first { and put in additionalClause
                            let codeIndex = code.indexOf("{");
                            code = code.substring(0, codeIndex+1) +"\t" + additionalClause + code.substring(codeIndex+1);
                            let newnode = parser.parse(code).rootNode;
                            let copyState = {...state};
                            transcompilationResult = scanClosure(newnode.children[0], "compound_statement", copyState, update);
                        }
                    }
                    transcompilationResult = [...additionalHeader, ...transcompilationResult];
                    transcompilationResult = [transcompilationResult.join("\n")];
                }else{
                    console.log("error : repeat_statement");
                }
                //to be continued       
            }
            break;
        case "if_expression":
            if(true){
                console.log("block");

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
                                copyState.scope = state.scope;
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
                        case "ERROR":
                            if(node.type === "assignment_expression"){
                                //regular expression for numbers
                                let reg = /^[0-9]+$/;
                                if(reg.test(childNode.text)){
                                    transcompilationResult = [...transcompilationResult,  childNode.text, "*"];
                                    break;
                                }
                            }
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
                                transcompilationResult = [...transcompilationResult , ...scanClosure(childNode, childNode.type, state, update)];
                            }
                            break;
                        default:
                            if(equalSymbol){
                                transcompilationResult = [...transcompilationResult , ...scanClosure(childNode, childNode.type, state, update)];
                            }else{
                                transcompilationResult = [...transcompilationResult , ...scanClosure(childNode, childNode.type, state, newUpdate)];
                            }
                            break;
                    }
                }
                if(state.topLevel){
                    for(const x of newUpdate.sourceControl.declaredIdentifiers){
                        update.sourceControl.declaredIdentifiers.push(x);
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
                copyState.scope = state.scope;
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
            findUsingIdentifier(node, state, update);
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
                if(sourceControlSwitch){
                    findUsingIdentifier(node, state, update);
                }
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
                                if(sourceControlSwitch){
                                    update.sourceControl.declaredIdentifiers.push(name);
                                }
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
                        case "identifier":
                            if(sourceControlSwitch){
                                update.sourceControl.declaredIdentifiers.push(childNode.text);
                            }
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
            if(state.sourceControlName === ""){
                let copyState = {...state};
                copyState.scope = state.scope;
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
            }else{
                let copyState = {...state};
                copyState.scope = state.scope;
                copyState.indent++;
                for(const childNode of node.children){
                    switch(childNode.type){
                        case "function_definition":
                            //see if  childNode.children[0].type contain "type"
                            //substring test
                            
                            if(contain("type", childNode.children[0].type) && !contain("operator", childNode.children[1].children[0].type)){
                                let newupdate = new UpDates();
                                scanFunctionForSourceControl(childNode, copyState, newupdate);
                                //we have assumed sourceControl over it
                                scanClosure(childNode, childNode.type, copyState, newupdate);
                                writeSource(newupdate.sourceControl);
                            }else{
                                if(childNode.children[0].type === "storage_class_specifier"){
                                    if(contain("type", childNode.children[1].type) && !contain("operator", childNode.children[2].children[0].type)){
                                        let newupdate = new UpDates();
                                        scanFunctionForSourceControl(childNode, copyState, newupdate);
                                        //we have assumed sourceControl over it
                                        scanClosure(childNode, childNode.type, copyState, newupdate);
                                        writeSource(newupdate.sourceControl);
                                    }else{
                                        scanClosure(childNode, childNode.type, copyState, update);
                                        transcompilationResult = [...transcompilationResult , childNode.text];        
                                    }
                                }else{
                                    scanClosure(childNode, childNode.type, copyState, update);
                                    transcompilationResult = [...transcompilationResult , childNode.text];    
                                }
                            }
                            break;
                        case "template_declaration":
                            //check if it is a function
                            if(childNode.children[2].type === "function_definition"){
                                let newupdate = new UpDates();
                                scanFunctionForSourceControl(childNode, copyState, newupdate);
                                //we have assumed sourceControl over it
                                scanClosure(childNode, childNode.type, copyState, newupdate);
                                writeSource(newupdate.sourceControl);
                                break;
                            }else{
                                scanClosure(childNode, childNode.type, copyState, update);
                                transcompilationResult = [...transcompilationResult , childNode.text];
                            }
                        default:
                            //transcompilationResult = [...transcompilationResult , ...scanClosure(childNode, childNode.type, copyState, update)];
                            scanClosure(childNode, childNode.type, copyState, update);
                            transcompilationResult = [...transcompilationResult , childNode.text];
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
               
                
            }
            break;
        case "namespace_definition":
        case "struct_specifier":
            if(node.text.startsWith("struct Comb")){
                console.log('here');
            }
        case "class_specifier":
            if(true){
                let structName = "";
                for(const childNode of node.children){
                    switch(childNode.type){
                        case "type_identifier":
                            if(structName === ""){
                                structName = childNode.text;
                                let copyState = {...state};
                                copyState.currentcontext.push(structName);
                                transcompilationResult[0] = transcompilationResult[0] + " " + structName;
                            }else{
                                transcompilationResult = [...transcompilationResult , ...scanClosure(childNode, childNode.type, state, update)];
                            }
                            break;
                        default:
                            transcompilationResult = [...transcompilationResult , ...scanClosure(childNode, childNode.type, state, update)];
                            break;
                    }
                }
                if(sourceControlSwitch ){
                    if(state.sourceControlName !== ""){
                        update.sourceControl.sourceCode = transcompilationResult;
                        update.sourceControl.declaredIdentifiers.push(structName);    
                    }else{
                        update.sourceControl.declaredIdentifiers.push(structName);
                    }
                }
                transcompilationResult.push(";"); 
                let bodyText = transcompilationResult.join(" ");
                transcompilationResult = [bodyText];
                state.currentcontext.pop();
            }
            break;
        case "rep_perm":
        case "rep_mcomb":
        case "rep_scomb":
        case "rep_marr":
        case "rep_sarr":
        case "rep_dist":
        case "rep":
        case "rrep":
        case "REP":
        case "RREP":
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
        case ">?=":
        case "<?=":
        case "%%=":
        case "**=":
        case "%%":
        case "**":
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
            if(true){

                for(let i = 0; i < node.children.length; i++){
                    const childNode = node.children[i];
                    switch(childNode.type){
                        case "comment":
                            if(true){
                                if(sourceControlSwitch){
                                    let interpretation = interpretComment(childNode, state, update);
                                    if(interpretation === true && childNode.text.startsWith("//CodeBlockName:")){
                                        if(childNode.text.startsWith("//CodeBlockName:Class")){
                                            let classname = childNode.text.split(":")[1];
                                            state.sourceControlName = classname;
                                            for(let j = i+1; j < node.children.length; j++){
                                                const childNode2 = node.children[j];
                                                if(childNode2.type === "comment"){
                                                    if(childNode2.text.startsWith("//CodeBlockName:")){
                                                        i = j-1;
                                                        break;
                                                    }
                                                    interpretation = interpretComment(childNode2, state, update);
                                                }else{
                                                    scanClosure(childNode2, childNode2.type, state, update);
                                                }
                                            }
                                            state.sourceControlName = "";
                                        }else{
                                            
                                            for(let j = i+1; j < node.children.length; ++j){
                                                const childNode2 = node.children[j];
                                                if(childNode2.type === "comment"){
                                                    if(childNode2.text.startsWith("//CodeBlockName:")){
                                                        break;
                                                    }
                                                    continue;
                                                }
                                                update.sourceControl.sourceCode.push(childNode2.text);
                                            }
                                        }
                                    }    
                                }
                            }
                            break;
                        case "declaration":
                            if(true){
                                let nstate = {...state};
                                nstate.topLevel = true;
                                transcompilationResult = [...transcompilationResult , ...scanClosure(childNode, childNode.type, nstate, update)];
                                nstate.topLevel = false;

                            }
                            break;
                        default:
                            transcompilationResult = [...transcompilationResult , ...scanClosure(childNode, childNode.type, state, update)];
                            break;
                    }
                }        
                if(!sourceControlSwitch){
                    for(const [key, value] of update.sourceControl.dependencies){   
                        allDependencies.set(key, value);
                    }
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
        case "user_defined_literal":
            // we want to match if it is [0-9]+d[0-9]+ 
            let re = new RegExp('[0-9]+d[0-9]+');
            if(re.test(node.text)){
                //split around d,
                let split = node.text.split("d");
                //get the first part
                let firstPart = split[0];
                //get the second part
                let secondPart = split[1];
                //convert second part into a number
                let secondPartNumber = parseInt(secondPart);
                for(let i = 0; i < secondPartNumber; ++i){
                    firstPart += "0";
                }
                transcompilationResult = [firstPart];
            }else{
                let se = new RegExp('[0-9]+[0-9a-zA-Z]+');
                if(se.test(node.text)){
                    let parts = [""];
                    let u = node.text;
                    for(let i = 0; i < u.length; ++i){
                        //check if u[i] is a digit
                        if(u[i] >= "0" && u[i] <= "9"){
                            if(parts.length === 1){
                                parts[0] += u[i];
                            }else{
                                parts[1] += u[i];
                            }
                        }else{
                            parts.push("");
                        }
                    }
                    transcompilationResult = [parts[0] + "*" + parts[1]];
                }else{ 
                    console.log("Error: user_defined_literal not implemented:" + node.text);
                }
            }


            break;
		default:
            if(category === "ERROR"){
                if(node.text === ";"){
                    console.log("error");
                }
                console.log( "ERROR: " + node.text);
                console.log(" Parent of Error" + node.parent?.text);
            }

            console.log(category, "+++" , node.text);
            break;
	}
	
	return transcompilationResult;
}

async function readParser(){
    let mapSC = new Map<string, SourceControl>();
    if(!sourceControlSwitch){
        const dependencies = fs.readFileSync("C:\\Users\\niyou\\Documents\\parserTest\\dependencies.json", "utf8");
        const dependenciesJson : SC[] = JSON.parse(dependencies);
        for(const dependency of dependenciesJson){
            //convert SC to sourceControl
            const sourceControl : SourceControl = {
                sourceCode : dependency.sourceCode,
                priorities : dependency.priorities,
                declaredIdentifiers : dependency.identifiers,
                name : dependency.name,
                dependencies : new Map<string, number> (),
                type : dependency.type,
                context : dependency.context
            };
            for(const name of dependency.dependencies){
                sourceControl.dependencies.set(name, 1);
                    let parentDependencies = mapSC.get(name)?.dependencies;
                    if(!parentDependencies){
                        continue;
                    }
                    for(const [key, val] of parentDependencies){
                    {
                        sourceControl.dependencies.set(key, val);
                    }
            }
        }
            mapSC.set(dependency.name, sourceControl);
            blockSC.push(sourceControl);
        }
    }
    await Parser.init();
    parser = new Parser();
    const lang = await Parser.Language.load(webAssemblyFile);
    parser.setLanguage(lang);
    //read from test.cpp file
    const file = fs.readFileSync("C:\\Users\\niyou\\Documents\\parserTest\\test.ocl", "utf8");
    const tree = parser.parse(file);
    const rootNode = tree.rootNode;
    const aggregateResult_ = scanTopLevel(rootNode, "main.ocl");
    const result = [];
    result.push("#pragma GCC optimize(\"Ofast\")");
    result.push("#pragma GCC optimize(\"unroll-loops\")");
    result.push("#pragma GCC optimize(\"inline\")");
    result.push("#include<bits/stdc++.h>");
    result.push("using namespace std;");
    result.push("");
    //compute dependencies closure:
    let usedDependencies = new Set<string>();
    for(const [key, value] of allDependencies){
        usedDependencies.add(key);
        let parentDependencies = mapSC.get(key)?.dependencies;
        if(!parentDependencies){
            continue;
        }
        for(const [ke, val] of parentDependencies){
            usedDependencies.add(ke);
        }
    }
    //convert usedDepenencies to array
    let usedDependenciesArray = Array.from(usedDependencies);
    //sort usedDependenciesArray based on priorities
    usedDependenciesArray.sort((a, b) => {
        let aPriority = mapSC.get(a)?.priorities;
        let bPriority = mapSC.get(b)?.priorities;
        if(aPriority === undefined || bPriority === undefined){
            return 0;
        }
        return aPriority - bPriority;
    });

    
    //console.log(usedDependencies);
    let frontCode = "";
    for(const key of usedDependenciesArray){
        let res =  mapSC.get(key)?.sourceCode.join("\n");
        let testFront = mapSC.get(key)?.type;
        if(testFront === "front"){
            if(res){
                frontCode += res + "\n";
            }                
        }
    }
    if(true){
        const tree = parser.parse(frontCode);
        const rootNode = tree.rootNode;  
        let frontResult_ = scanTopLevel(rootNode, "root.ocl");     
        for(const line of frontResult_){
            if(line.length > 0){
                result.push(line);
            }
        } 
    }
    

    for(const line of aggregateResult_){
        if(line.length > 0){
            result.push(line);
        }
    }
    //write result to output.cpp file with all the lines
    fs.writeFileSync("C:\\Users\\niyou\\Documents\\parserTest\\output.cpp", result.join("\n"));
    if(sourceControlSwitch){
        let blockSCC : SC[] = [];
        let priorities = 0;
        for(const x of blockSC){
            let dependenciesRET = [];
            for(const dependency of x.dependencies){
                dependenciesRET.push(dependency[0]);
            }
            
            blockSCC.push({
                name : x.name,
                sourceCode : processIt(x.sourceCode),
                dependencies : dependenciesRET,
                type: x.type,
                identifiers : x.declaredIdentifiers,
                priorities : priorities,
                context : x.context
            });
            priorities++;

        }
        let retu = JSON.stringify(blockSCC, null, '\t');
        fs.writeFileSync("C:\\Users\\niyou\\Documents\\parserTest\\test.json", retu);
    }

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


function interpretComment(node: Parser.SyntaxNode, state: State, update: UpDates) {
    let text = node.text;
    text = text.substring(2);
    if(text.startsWith("CodeBlockName:")){
        let name = text.substring(14);
        if(update.sourceControl.name !== ""){
            writeSource(update.sourceControl);
            update.sourceControl = new SourceControl();
        }
        update.sourceControl.name = name.replace("\r", "");
        return true;

    }else if(text.startsWith("CodeBlockType:")){
        let type = text.substring(14);
        update.sourceControl.type = type.replace("\r", "");
        return true;
    }
    return false;
}


function processIt(astri :Array<string>) : Array<string>{
    let ret : string[] = [];
    for(const text of astri){
        ret = [...ret, ...text.split("\r\n")];
    }

    return ret;
}

function writeSource(sourceControl: SourceControl) {
    let dependenciesRET = [];
    for(const dependency of sourceControl.dependencies){
        dependenciesRET.push(dependency[0]);
    }
    blockSC.push(sourceControl);

}

function findUsingIdentifier(node: Parser.SyntaxNode, state: State, update: UpDates) {
    let identifierName = node.text;
    let currentContext : string[] = [];
    for(let i = 0; i <= state.currentcontext.length; ++i){
        for(const x of blockSC){
            if(x.name === update.sourceControl.name){
                continue;
            }
            if(x.context.join("::") !== currentContext.join("::")){
                continue;
            }
            for(const y of x.declaredIdentifiers){
                if(y === identifierName){
                    update.sourceControl.dependencies.set(x.name, 1);
                }
            }
        }
        if(i === state.currentcontext.length){
            break;
        }
        currentContext.push(state.currentcontext[i]);
    }
}

function findFirstNameByFunctionDefinition(node: Parser.SyntaxNode, state: State, update: UpDates) {
    let ret = "";
    for(const childrenNode of node.children){
        switch(childrenNode.type){
            case "function_declarator":
                ret = childrenNode.children[0].text;
                break;
            default:
                break;
        };
    }
    return ret;
}

function findFirstNameByTemplateDefinition(node: Parser.SyntaxNode, state : State, update : UpDates){
    let ret = "";
    for(const childrenNode of node.children){
        switch(childrenNode.type){
            case "function_definition":
                ret = findFirstNameByFunctionDefinition(childrenNode, state, update);
                break;
            default:
                break;
        };
    }
    return ret;
}

function scanFunctionForSourceControl(childNode: Parser.SyntaxNode, copyState: State, update: UpDates) {
    let ret = "";
    let firstName = "";
    if(childNode.type === "function_definition"){
        firstName = findFirstNameByFunctionDefinition(childNode, copyState, update);
    }
    if(childNode.type === "template_declaration"){
        firstName = findFirstNameByTemplateDefinition(childNode, copyState, update);
    }

    for(const childrenNode of childNode.children){
        switch(childrenNode.type){
            default:
                ret += childrenNode.text;
                ret += " ";
                break;
        };
    }
    update.sourceControl.name = copyState.sourceControlName.replace("\r", "")+ "::" + firstName;
    update.sourceControl.context = [];
    for(const x of copyState.currentcontext){
        update.sourceControl.context.push(x);
    }
    update.sourceControl.type = "front:" + copyState.sourceControlName.replace("\r", "");
    update.sourceControl.declaredIdentifiers.push(firstName);
    update.sourceControl.sourceCode = ret.split("\r\n");

}

function contain(substring: string, text: string) : boolean {
    if(text.indexOf(substring) !== -1){
        return true;
    }
    return false;
}

function echoReplace(childNode: Parser.SyntaxNode, state: State, update: UpDates, replaceFrom: string, replaceTo: string) : string {
    let currentText = childNode.text;
    let cnt = 0;
    let retText = "";
    for(let i = 0; i < currentText.length; ++i){
        if(currentText[i] === '(' || currentText[i] === '[' || currentText === '{'){
            ++cnt;
        }
        if(currentText[i] === ')' || currentText[i] === ']' || currentText === '}'){
            --cnt;
        }
        if(currentText[i] === replaceFrom && cnt === 0){
            retText += replaceTo;
        }else{
            retText += currentText[i];
        }
    }
    return retText ;
}

