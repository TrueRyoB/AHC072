/* @ts-self-types="./BrsBAXF2.d.ts" */

export class GenOption {
    __destroy_into_raw() {
        const ptr = this.__wbg_ptr;
        this.__wbg_ptr = 0;
        GenOptionFinalization.unregister(this);
        return ptr;
    }
    free() {
        const ptr = this.__destroy_into_raw();
        __wbg_call_guard();
        wasm.__wbg_genoption_free(ptr, 0);

    }
    constructor() {
        let ret;
        __wbg_call_guard();
        ret = wasm.genoption_new();
        this.__wbg_ptr = ret;
        Object.defineProperty(this, '__wbg_inst', { value: __wbg_instance_id, writable: true });
        GenOptionFinalization.register(this, { ptr: ret, instance: __wbg_instance_id }, this);
        return this;
    }
    /**
     * @param {string} name
     * @param {number} value
     */
    set_f64(name, value) {
        if (this.__wbg_inst !== undefined && this.__wbg_inst !== __wbg_instance_id) {
            throw new Error('Invalid stale object from previous Wasm instance');
        }
        const ptr0 = passStringToWasm0(name, wasm.__wbindgen_malloc, wasm.__wbindgen_realloc);
        const len0 = WASM_VECTOR_LEN;
        let ret;
        __wbg_call_guard();
        ret = wasm.genoption_set_f64(this.__wbg_ptr, ptr0, len0, value);
        if (ret[1]) {
            throw takeFromExternrefTable0(ret[0]);
        }
    }
    /**
     * 問題固有の整数パラメータを固定する。名前と処理は tools/src/lib.rs にだけ追加する。
     * @param {string} name
     * @param {number} value
     */
    set_i32(name, value) {
        if (this.__wbg_inst !== undefined && this.__wbg_inst !== __wbg_instance_id) {
            throw new Error('Invalid stale object from previous Wasm instance');
        }
        const ptr0 = passStringToWasm0(name, wasm.__wbindgen_malloc, wasm.__wbindgen_realloc);
        const len0 = WASM_VECTOR_LEN;
        let ret;
        __wbg_call_guard();
        ret = wasm.genoption_set_i32(this.__wbg_ptr, ptr0, len0, value);
        if (ret[1]) {
            throw takeFromExternrefTable0(ret[0]);
        }
    }
}
if (Symbol.dispose) GenOption.prototype[Symbol.dispose] = GenOption.prototype.free;

export class Ret {
    static __wrap(ptr) {
        const obj = Object.create(Ret.prototype);
        obj.__wbg_ptr = ptr;
        Object.defineProperty(obj, '__wbg_inst', { value: __wbg_instance_id, writable: true });

        RetFinalization.register(obj, { ptr, instance: __wbg_instance_id }, obj);
        return obj;
    }
    __destroy_into_raw() {
        const ptr = this.__wbg_ptr;
        this.__wbg_ptr = 0;
        RetFinalization.unregister(this);
        return ptr;
    }
    free() {
        const ptr = this.__destroy_into_raw();
        __wbg_call_guard();
        wasm.__wbg_ret_free(ptr, 0);

    }
    /**
     * @returns {string}
     */
    get svg() {
        let deferred1_0;
        let deferred1_1;
        try {
            if (this.__wbg_inst !== undefined && this.__wbg_inst !== __wbg_instance_id) {
                throw new Error('Invalid stale object from previous Wasm instance');
            }
            let ret;
            __wbg_call_guard();
            ret = wasm.__wbg_get_ret_svg(this.__wbg_ptr);
            deferred1_0 = ret[0];
            deferred1_1 = ret[1];
            return getStringFromWasm0(ret[0], ret[1]);
        } finally {
            __wbg_call_guard();
            wasm.__wbindgen_free(deferred1_0, deferred1_1, 1);
        }
    }
    /**
     * @param {string} arg0
     */
    set svg(arg0) {
        if (this.__wbg_inst !== undefined && this.__wbg_inst !== __wbg_instance_id) {
            throw new Error('Invalid stale object from previous Wasm instance');
        }
        const ptr0 = passStringToWasm0(arg0, wasm.__wbindgen_malloc, wasm.__wbindgen_realloc);
        const len0 = WASM_VECTOR_LEN;
        __wbg_call_guard();
        wasm.__wbg_set_ret_svg(this.__wbg_ptr, ptr0, len0);
    }
}
if (Symbol.dispose) Ret.prototype[Symbol.dispose] = Ret.prototype.free;

export class VisOption {
    __destroy_into_raw() {
        const ptr = this.__wbg_ptr;
        this.__wbg_ptr = 0;
        VisOptionFinalization.unregister(this);
        return ptr;
    }
    free() {
        const ptr = this.__destroy_into_raw();
        __wbg_call_guard();
        wasm.__wbg_visoption_free(ptr, 0);

    }
    constructor() {
        let ret;
        __wbg_call_guard();
        ret = wasm.visoption_new();
        this.__wbg_ptr = ret;
        Object.defineProperty(this, '__wbg_inst', { value: __wbg_instance_id, writable: true });
        VisOptionFinalization.register(this, { ptr: ret, instance: __wbg_instance_id }, this);
        return this;
    }
    /**
     * 問題固有の真偽値オプションを設定する。名前と処理は tools/src/vis.rs にだけ追加する。
     * @param {string} name
     * @param {boolean} value
     */
    set_bool(name, value) {
        if (this.__wbg_inst !== undefined && this.__wbg_inst !== __wbg_instance_id) {
            throw new Error('Invalid stale object from previous Wasm instance');
        }
        const ptr0 = passStringToWasm0(name, wasm.__wbindgen_malloc, wasm.__wbindgen_realloc);
        const len0 = WASM_VECTOR_LEN;
        let ret;
        __wbg_call_guard();
        ret = wasm.visoption_set_bool(this.__wbg_ptr, ptr0, len0, value);
        if (ret[1]) {
            throw takeFromExternrefTable0(ret[0]);
        }
    }
    /**
     * 問題固有の整数オプションを設定する。名前と処理は tools/src/vis.rs にだけ追加する。
     * @param {string} name
     * @param {number} value
     */
    set_i32(name, value) {
        if (this.__wbg_inst !== undefined && this.__wbg_inst !== __wbg_instance_id) {
            throw new Error('Invalid stale object from previous Wasm instance');
        }
        const ptr0 = passStringToWasm0(name, wasm.__wbindgen_malloc, wasm.__wbindgen_realloc);
        const len0 = WASM_VECTOR_LEN;
        let ret;
        __wbg_call_guard();
        ret = wasm.visoption_set_i32(this.__wbg_ptr, ptr0, len0, value);
        if (ret[1]) {
            throw takeFromExternrefTable0(ret[0]);
        }
    }
    /**
     * 問題固有の選択肢を文字列で設定する。名前と処理は tools/src/vis.rs にだけ追加する。
     * @param {string} name
     * @param {string} value
     */
    set_string(name, value) {
        if (this.__wbg_inst !== undefined && this.__wbg_inst !== __wbg_instance_id) {
            throw new Error('Invalid stale object from previous Wasm instance');
        }
        const ptr0 = passStringToWasm0(name, wasm.__wbindgen_malloc, wasm.__wbindgen_realloc);
        const len0 = WASM_VECTOR_LEN;
        const ptr1 = passStringToWasm0(value, wasm.__wbindgen_malloc, wasm.__wbindgen_realloc);
        const len1 = WASM_VECTOR_LEN;
        let ret;
        __wbg_call_guard();
        ret = wasm.visoption_set_string(this.__wbg_ptr, ptr0, len0, ptr1, len1);
        if (ret[1]) {
            throw takeFromExternrefTable0(ret[0]);
        }
    }
    /**
     * Web のスライダーで選んだ表示位置を設定する。
     * @param {number} t
     */
    set_t(t) {
        if (this.__wbg_inst !== undefined && this.__wbg_inst !== __wbg_instance_id) {
            throw new Error('Invalid stale object from previous Wasm instance');
        }
        __wbg_call_guard();
        wasm.visoption_set_t(this.__wbg_ptr, t);
    }
}
if (Symbol.dispose) VisOption.prototype[Symbol.dispose] = VisOption.prototype.free;

/**
 * 一組の入出力を一度だけパース・評価し、描画用データを保持する。
 */
export class Visualizer {
    static __wrap(ptr) {
        const obj = Object.create(Visualizer.prototype);
        obj.__wbg_ptr = ptr;
        Object.defineProperty(obj, '__wbg_inst', { value: __wbg_instance_id, writable: true });

        VisualizerFinalization.register(obj, { ptr, instance: __wbg_instance_id }, obj);
        return obj;
    }
    __destroy_into_raw() {
        const ptr = this.__wbg_ptr;
        this.__wbg_ptr = 0;
        VisualizerFinalization.unregister(this);
        return ptr;
    }
    free() {
        const ptr = this.__destroy_into_raw();
        __wbg_call_guard();
        wasm.__wbg_visualizer_free(ptr, 0);

    }
    /**
     * @param {number} t
     * @param {number} p
     * @returns {number}
     */
    manual_height(t, p) {
        if (this.__wbg_inst !== undefined && this.__wbg_inst !== __wbg_instance_id) {
            throw new Error('Invalid stale object from previous Wasm instance');
        }
        let ret;
        __wbg_call_guard();
        ret = wasm.visualizer_manual_height(this.__wbg_ptr, t, p);
        return ret >>> 0;
    }
    /**
     * @param {number} t
     * @param {number} p
     * @param {number} k
     * @param {number} q
     * @returns {string}
     */
    manual_move(t, p, k, q) {
        let deferred2_0;
        let deferred2_1;
        try {
            if (this.__wbg_inst !== undefined && this.__wbg_inst !== __wbg_instance_id) {
                throw new Error('Invalid stale object from previous Wasm instance');
            }
            let ret;
            __wbg_call_guard();
            ret = wasm.visualizer_manual_move(this.__wbg_ptr, t, p, k, q);
            var ptr1 = ret[0];
            var len1 = ret[1];
            if (ret[3]) {
                ptr1 = 0; len1 = 0;
                throw takeFromExternrefTable0(ret[2]);
            }
            deferred2_0 = ptr1;
            deferred2_1 = len1;
            return getStringFromWasm0(ptr1, len1);
        } finally {
            __wbg_call_guard();
            wasm.__wbindgen_free(deferred2_0, deferred2_1, 1);
        }
    }
    /**
     * @param {number} t
     * @param {number} p
     * @param {number} k
     * @returns {string}
     */
    manual_overlay(t, p, k) {
        let deferred1_0;
        let deferred1_1;
        try {
            if (this.__wbg_inst !== undefined && this.__wbg_inst !== __wbg_instance_id) {
                throw new Error('Invalid stale object from previous Wasm instance');
            }
            let ret;
            __wbg_call_guard();
            ret = wasm.visualizer_manual_overlay(this.__wbg_ptr, t, p, k);
            deferred1_0 = ret[0];
            deferred1_1 = ret[1];
            return getStringFromWasm0(ret[0], ret[1]);
        } finally {
            __wbg_call_guard();
            wasm.__wbindgen_free(deferred1_0, deferred1_1, 1);
        }
    }
    /**
     * 手動操作は表示中の最後の合法状態を起点にする。
     * @param {number} t
     * @returns {number}
     */
    manual_turn(t) {
        if (this.__wbg_inst !== undefined && this.__wbg_inst !== __wbg_instance_id) {
            throw new Error('Invalid stale object from previous Wasm instance');
        }
        let ret;
        __wbg_call_guard();
        ret = wasm.visualizer_manual_turn(this.__wbg_ptr, t);
        return ret >>> 0;
    }
    /**
     * @param {number} t
     * @returns {string}
     */
    manual_undo(t) {
        let deferred1_0;
        let deferred1_1;
        try {
            if (this.__wbg_inst !== undefined && this.__wbg_inst !== __wbg_instance_id) {
                throw new Error('Invalid stale object from previous Wasm instance');
            }
            let ret;
            __wbg_call_guard();
            ret = wasm.visualizer_manual_undo(this.__wbg_ptr, t);
            deferred1_0 = ret[0];
            deferred1_1 = ret[1];
            return getStringFromWasm0(ret[0], ret[1]);
        } finally {
            __wbg_call_guard();
            wasm.__wbindgen_free(deferred1_0, deferred1_1, 1);
        }
    }
    /**
     * @param {VisOption} option
     * @returns {number}
     */
    max_turn(option) {
        if (this.__wbg_inst !== undefined && this.__wbg_inst !== __wbg_instance_id) {
            throw new Error('Invalid stale object from previous Wasm instance');
        }
        _assertClass(option, VisOption);
        if ((option).__wbg_inst !== undefined && (option).__wbg_inst !== __wbg_instance_id) {
            throw new Error('Invalid stale object from previous Wasm instance');
        }
        let ret;
        __wbg_call_guard();
        ret = wasm.visualizer_max_turn(this.__wbg_ptr, option.__wbg_ptr);
        return ret;
    }
    /**
     * @param {string} input
     * @param {string} output
     */
    constructor(input, output) {
        const ptr0 = passStringToWasm0(input, wasm.__wbindgen_malloc, wasm.__wbindgen_realloc);
        const len0 = WASM_VECTOR_LEN;
        const ptr1 = passStringToWasm0(output, wasm.__wbindgen_malloc, wasm.__wbindgen_realloc);
        const len1 = WASM_VECTOR_LEN;
        let ret;
        __wbg_call_guard();
        ret = wasm.visualizer_new(ptr0, len0, ptr1, len1);
        this.__wbg_ptr = ret;
        Object.defineProperty(this, '__wbg_inst', { value: __wbg_instance_id, writable: true });
        VisualizerFinalization.register(this, { ptr: ret, instance: __wbg_instance_id }, this);
        return this;
    }
    /**
     * @param {VisOption} option
     * @returns {Ret}
     */
    render(option) {
        if (this.__wbg_inst !== undefined && this.__wbg_inst !== __wbg_instance_id) {
            throw new Error('Invalid stale object from previous Wasm instance');
        }
        _assertClass(option, VisOption);
        if ((option).__wbg_inst !== undefined && (option).__wbg_inst !== __wbg_instance_id) {
            throw new Error('Invalid stale object from previous Wasm instance');
        }
        let ret;
        __wbg_call_guard();
        ret = wasm.visualizer_render(this.__wbg_ptr, option.__wbg_ptr);
        return Ret.__wrap(ret);
    }
    /**
     * 非同期の画像・動画生成が現在の入出力を保持するための独立したハンドル。
     * @returns {Visualizer}
     */
    snapshot() {
        if (this.__wbg_inst !== undefined && this.__wbg_inst !== __wbg_instance_id) {
            throw new Error('Invalid stale object from previous Wasm instance');
        }
        let ret;
        __wbg_call_guard();
        ret = wasm.visualizer_snapshot(this.__wbg_ptr);
        return Visualizer.__wrap(ret);
    }
}
if (Symbol.dispose) Visualizer.prototype[Symbol.dispose] = Visualizer.prototype.free;

function __wbg_reset_state () {
    __wbg_instance_id++;
    cachedUint8ArrayMemory0 = null;
    if (typeof numBytesDecoded !== 'undefined') numBytesDecoded = 0;
    if (typeof WASM_VECTOR_LEN !== 'undefined') WASM_VECTOR_LEN = 0;
    __wbg_reinit_scheduled = false;
    wasmInstance = new WebAssembly.Instance(wasmModule, __wbg_get_imports());
    wasm = wasmInstance.exports;
    wasm.__wbindgen_start();
}

/**
 * @param {string} seed
 * @param {GenOption} option
 * @returns {string}
 */
export function gen(seed, option) {
    let deferred2_0;
    let deferred2_1;
    try {
        const ptr0 = passStringToWasm0(seed, wasm.__wbindgen_malloc, wasm.__wbindgen_realloc);
        const len0 = WASM_VECTOR_LEN;
        _assertClass(option, GenOption);
        if ((option).__wbg_inst !== undefined && (option).__wbg_inst !== __wbg_instance_id) {
            throw new Error('Invalid stale object from previous Wasm instance');
        }
        let ret;
        __wbg_call_guard();
        ret = wasm.gen(ptr0, len0, option.__wbg_ptr);
        deferred2_0 = ret[0];
        deferred2_1 = ret[1];
        return getStringFromWasm0(ret[0], ret[1]);
    } finally {
        __wbg_call_guard();
        wasm.__wbindgen_free(deferred2_0, deferred2_1, 1);
    }
}

/**
 * @returns {string}
 */
export function get_sample_output() {
    let deferred1_0;
    let deferred1_1;
    try {
        let ret;
        __wbg_call_guard();
        ret = wasm.get_sample_output();
        deferred1_0 = ret[0];
        deferred1_1 = ret[1];
        return getStringFromWasm0(ret[0], ret[1]);
    } finally {
        __wbg_call_guard();
        wasm.__wbindgen_free(deferred1_0, deferred1_1, 1);
    }
}

/**
 * panic 後の借用とメモリを破棄するため、次の呼び出しで Wasm を作り直す。
 */
export function reset_wasm() {
    __wbg_call_guard();
    wasm.reset_wasm();
}

/**
 * @param {string} input
 * @param {string} output
 * @returns {bigint}
 */
export function share_score(input, output) {
    const ptr0 = passStringToWasm0(input, wasm.__wbindgen_malloc, wasm.__wbindgen_realloc);
    const len0 = WASM_VECTOR_LEN;
    const ptr1 = passStringToWasm0(output, wasm.__wbindgen_malloc, wasm.__wbindgen_realloc);
    const len1 = WASM_VECTOR_LEN;
    let ret;
    __wbg_call_guard();
    ret = wasm.share_score(ptr0, len0, ptr1, len1);
    if (ret[2]) {
        throw takeFromExternrefTable0(ret[1]);
    }
    return ret[0];
}
function __wbg_get_imports() {
    const import0 = {
        __proto__: null,
        __wbg_Error_408e67f47ca7b58b: function(arg0, arg1) {
            const ret = Error(getStringFromWasm0(arg0, arg1));
            return ret;
        },
        __wbg___wbindgen_reinit_eaa1836ea9a8a649: function() {
            __wbg_reinit_scheduled = true;
        },
        __wbg___wbindgen_throw_bb96b2010945f0bc: function(arg0, arg1) {
            throw new Error(getStringFromWasm0(arg0, arg1));
        },
        __wbindgen_init_externref_table: function() {
            const table = wasm.__wbindgen_externrefs;
            const offset = table.grow(4);
            table.set(0, undefined);
            table.set(offset + 0, undefined);
            table.set(offset + 1, null);
            table.set(offset + 2, true);
            table.set(offset + 3, false);
        },
    };
    return {
        __proto__: null,
        "./BrsBAXF2_bg.js": import0,
    };
}

function __wbg_call_guard() {
    if (__wbg_reinit_scheduled) {
        __wbg_reset_state();
        return;
    }
}


let __wbg_instance_id = 0;
const GenOptionFinalization = (typeof FinalizationRegistry === 'undefined')
    ? { register: () => {}, unregister: () => {} }
    : new FinalizationRegistry(({ ptr, instance }) => {
    if (instance === __wbg_instance_id) wasm.__wbg_genoption_free(ptr, 1);
});
const RetFinalization = (typeof FinalizationRegistry === 'undefined')
    ? { register: () => {}, unregister: () => {} }
    : new FinalizationRegistry(({ ptr, instance }) => {
    if (instance === __wbg_instance_id) wasm.__wbg_ret_free(ptr, 1);
});
const VisOptionFinalization = (typeof FinalizationRegistry === 'undefined')
    ? { register: () => {}, unregister: () => {} }
    : new FinalizationRegistry(({ ptr, instance }) => {
    if (instance === __wbg_instance_id) wasm.__wbg_visoption_free(ptr, 1);
});
const VisualizerFinalization = (typeof FinalizationRegistry === 'undefined')
    ? { register: () => {}, unregister: () => {} }
    : new FinalizationRegistry(({ ptr, instance }) => {
    if (instance === __wbg_instance_id) wasm.__wbg_visualizer_free(ptr, 1);
});

function _assertClass(instance, klass) {
    if (!(instance instanceof klass)) {
        throw new Error(`expected instance of ${klass.name}`);
    }
}

function getStringFromWasm0(ptr, len) {
    return decodeText(ptr >>> 0, len);
}

let cachedUint8ArrayMemory0 = null;
function getUint8ArrayMemory0() {
    if (cachedUint8ArrayMemory0 === null || cachedUint8ArrayMemory0.byteLength === 0) {
        cachedUint8ArrayMemory0 = new Uint8Array(wasm.memory.buffer);
    }
    return cachedUint8ArrayMemory0;
}

function passStringToWasm0(arg, malloc, realloc) {
    if (realloc === undefined) {
        const buf = cachedTextEncoder.encode(arg);
        const ptr = malloc(buf.length, 1) >>> 0;
        getUint8ArrayMemory0().subarray(ptr, ptr + buf.length).set(buf);
        WASM_VECTOR_LEN = buf.length;
        return ptr;
    }

    let len = arg.length;
    let ptr = malloc(len, 1) >>> 0;

    const mem = getUint8ArrayMemory0();

    let offset = 0;

    for (; offset < len; offset++) {
        const code = arg.charCodeAt(offset);
        if (code > 0x7F) break;
        mem[ptr + offset] = code;
    }
    if (offset !== len) {
        if (offset !== 0) {
            arg = arg.slice(offset);
        }
        ptr = realloc(ptr, len, len = offset + arg.length * 3, 1) >>> 0;
        const view = getUint8ArrayMemory0().subarray(ptr + offset, ptr + len);
        const ret = cachedTextEncoder.encodeInto(arg, view);

        offset += ret.written;
        ptr = realloc(ptr, len, offset, 1) >>> 0;
    }

    WASM_VECTOR_LEN = offset;
    return ptr;
}

let __wbg_reinit_scheduled = false;

function takeFromExternrefTable0(idx) {
    const value = wasm.__wbindgen_externrefs.get(idx);
    wasm.__externref_table_dealloc(idx);
    return value;
}

let cachedTextDecoder = new TextDecoder('utf-8', { ignoreBOM: true, fatal: true });
cachedTextDecoder.decode();
const MAX_SAFARI_DECODE_BYTES = 2146435072;
let numBytesDecoded = 0;
function decodeText(ptr, len) {
    numBytesDecoded += len;
    if (numBytesDecoded >= MAX_SAFARI_DECODE_BYTES) {
        cachedTextDecoder = new TextDecoder('utf-8', { ignoreBOM: true, fatal: true });
        cachedTextDecoder.decode();
        numBytesDecoded = len;
    }
    return cachedTextDecoder.decode(getUint8ArrayMemory0().subarray(ptr, ptr + len));
}

const cachedTextEncoder = new TextEncoder();

if (!('encodeInto' in cachedTextEncoder)) {
    cachedTextEncoder.encodeInto = function (arg, view) {
        const buf = cachedTextEncoder.encode(arg);
        view.set(buf);
        return {
            read: arg.length,
            written: buf.length
        };
    };
}

let WASM_VECTOR_LEN = 0;

let wasmModule, wasmInstance, wasm;
function __wbg_finalize_init(instance, module) {
    wasmInstance = instance;
    wasm = instance.exports;
    wasmModule = module;
    cachedUint8ArrayMemory0 = null;
    wasm.__wbindgen_start();
    return wasm;
}

async function __wbg_load(module, imports) {
    if (typeof Response === 'function' && module instanceof Response) {
        if (!module.ok) {
            throw new Error(`failed to fetch Wasm: ${module.status} ${module.statusText} fetching '${module.url}'`);
        }

        if (typeof WebAssembly.instantiateStreaming === 'function') {
            try {
                return await WebAssembly.instantiateStreaming(module, imports);
            } catch (e) {
                const validResponse = expectedResponseType(module.type);

                if (validResponse && module.headers.get('Content-Type') !== 'application/wasm') {
                    console.warn("`WebAssembly.instantiateStreaming` failed because your server does not serve Wasm with `application/wasm` MIME type. Falling back to `WebAssembly.instantiate` which is slower. Original error:\n", e);

                } else { throw e; }
            }
        }

        const bytes = await module.arrayBuffer();
        return await WebAssembly.instantiate(bytes, imports);
    } else {
        const instance = await WebAssembly.instantiate(module, imports);

        if (instance instanceof WebAssembly.Instance) {
            return { instance, module };
        } else {
            return instance;
        }
    }

    function expectedResponseType(type) {
        switch (type) {
            case 'basic': case 'cors': case 'default': return true;
        }
        return false;
    }
}

function initSync(module) {
    if (wasm !== undefined) return wasm;


    if (module !== undefined) {
        if (Object.getPrototypeOf(module) === Object.prototype) {
            ({module} = module)
        } else {
            console.warn('using deprecated parameters for `initSync()`; pass a single object instead')
        }
    }

    const imports = __wbg_get_imports();
    if (!(module instanceof WebAssembly.Module)) {
        module = new WebAssembly.Module(module);
    }
    const instance = new WebAssembly.Instance(module, imports);
    return __wbg_finalize_init(instance, module);
}

async function __wbg_init(module_or_path) {
    if (wasm !== undefined) return wasm;


    if (module_or_path !== undefined) {
        if (Object.getPrototypeOf(module_or_path) === Object.prototype) {
            ({module_or_path} = module_or_path)
        } else {
            console.warn('using deprecated parameters for the initialization function; pass a single object instead')
        }
    }

    if (module_or_path === undefined) {
        module_or_path = new URL('BrsBAXF2_bg.wasm', import.meta.url);
    }
    const imports = __wbg_get_imports();

    if (typeof module_or_path === 'string' || (typeof Request === 'function' && module_or_path instanceof Request) || (typeof URL === 'function' && module_or_path instanceof URL)) {
        module_or_path = fetch(module_or_path);
    }

    const { instance, module } = await __wbg_load(await module_or_path, imports);

    return __wbg_finalize_init(instance, module);
}

export { initSync, __wbg_init as default };
