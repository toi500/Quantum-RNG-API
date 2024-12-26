#include <napi.h>

// Declare C linkage for quantum_rng functions
extern "C" {
#include "quantum_rng.h"
}

class QuantumRNG : public Napi::ObjectWrap<QuantumRNG> {
public:
    static Napi::Object Init(Napi::Env env, Napi::Object exports);
    QuantumRNG(const Napi::CallbackInfo& info);
    ~QuantumRNG();

private:
    static Napi::FunctionReference constructor;
    qrng_ctx* ctx;

    // Wrapped methods
    Napi::Value GetBytes(const Napi::CallbackInfo& info);
    Napi::Value GetUInt64(const Napi::CallbackInfo& info);
    Napi::Value GetDouble(const Napi::CallbackInfo& info);
    Napi::Value GetRange32(const Napi::CallbackInfo& info);
    Napi::Value GetRange64(const Napi::CallbackInfo& info);
    Napi::Value GetEntropyEstimate(const Napi::CallbackInfo& info);
    Napi::Value Reseed(const Napi::CallbackInfo& info);
    Napi::Value EntangleStates(const Napi::CallbackInfo& info);
    Napi::Value MeasureState(const Napi::CallbackInfo& info);
    static Napi::Value GetVersion(const Napi::CallbackInfo& info);
};

Napi::FunctionReference QuantumRNG::constructor;

Napi::Object QuantumRNG::Init(Napi::Env env, Napi::Object exports) {
    Napi::HandleScope scope(env);

    Napi::Function func = DefineClass(env, "QuantumRNG", {
        InstanceMethod("getBytes", &QuantumRNG::GetBytes),
        InstanceMethod("getUInt64", &QuantumRNG::GetUInt64),
        InstanceMethod("getDouble", &QuantumRNG::GetDouble),
        InstanceMethod("getRange32", &QuantumRNG::GetRange32),
        InstanceMethod("getRange64", &QuantumRNG::GetRange64),
        InstanceMethod("getEntropyEstimate", &QuantumRNG::GetEntropyEstimate),
        InstanceMethod("reseed", &QuantumRNG::Reseed),
        InstanceMethod("entangleStates", &QuantumRNG::EntangleStates),
        InstanceMethod("measureState", &QuantumRNG::MeasureState),
        StaticMethod("getVersion", &QuantumRNG::GetVersion)
    });

    constructor = Napi::Persistent(func);
    constructor.SuppressDestruct();

    exports.Set("QuantumRNG", func);
    return exports;
}

QuantumRNG::QuantumRNG(const Napi::CallbackInfo& info) 
    : Napi::ObjectWrap<QuantumRNG>(info), ctx(nullptr) {
    Napi::Env env = info.Env();
    try {
        fprintf(stderr, "QuantumRNG constructor start\n");
        
        if (info.Length() > 0 && !info[0].IsBuffer()) {
            fprintf(stderr, "Invalid seed type\n");
            throw Napi::TypeError::New(env, "Seed must be a Buffer");
        }

        const uint8_t* seed = nullptr;
        size_t seed_len = 0;

        if (info.Length() > 0) {
            fprintf(stderr, "Processing seed buffer\n");
            Napi::Buffer<uint8_t> seedBuffer = info[0].As<Napi::Buffer<uint8_t>>();
            seed = seedBuffer.Data();
            seed_len = seedBuffer.Length();
        }

        fprintf(stderr, "Calling qrng_init\n");
        qrng_error err = qrng_init(&ctx, seed, seed_len);
        fprintf(stderr, "qrng_init returned: %d\n", err);
        if (err != QRNG_SUCCESS) {
            if (ctx) {
                free(ctx);
                ctx = nullptr;
            }
            throw Napi::Error::New(env, qrng_error_string(err));
        }
    } catch (const Napi::Error& e) {
        e.ThrowAsJavaScriptException();
    }
}

QuantumRNG::~QuantumRNG() {
    if (ctx) {
        qrng_free(ctx);
        ctx = nullptr;
    }
}

Napi::Value QuantumRNG::GetBytes(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    if (info.Length() < 1 || !info[0].IsNumber()) {
        Napi::TypeError::New(env, "Number of bytes required").ThrowAsJavaScriptException();
        return env.Null();
    }

    size_t length = info[0].As<Napi::Number>().Uint32Value();
    Napi::Buffer<uint8_t> buffer = Napi::Buffer<uint8_t>::New(env, length);

    qrng_error err = qrng_bytes(ctx, buffer.Data(), length);
    if (err != QRNG_SUCCESS) {
        Napi::Error::New(env, qrng_error_string(err)).ThrowAsJavaScriptException();
        return env.Null();
    }

    return buffer;
}

Napi::Value QuantumRNG::GetUInt64(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    uint64_t value = qrng_uint64(ctx);
    return Napi::BigInt::New(env, value);
}

Napi::Value QuantumRNG::GetDouble(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    double value = qrng_double(ctx);
    return Napi::Number::New(env, value);
}

Napi::Value QuantumRNG::GetRange32(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    if (info.Length() < 2 || !info[0].IsNumber() || !info[1].IsNumber()) {
        Napi::TypeError::New(env, "Min and max values required").ThrowAsJavaScriptException();
        return env.Null();
    }

    int32_t min = info[0].As<Napi::Number>().Int32Value();
    int32_t max = info[1].As<Napi::Number>().Int32Value();

    int32_t value = qrng_range32(ctx, min, max);
    return Napi::Number::New(env, value);
}

Napi::Value QuantumRNG::GetRange64(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    if (info.Length() < 2) {
        Napi::TypeError::New(env, "Min and max values required").ThrowAsJavaScriptException();
        return env.Null();
    }

    bool lossless;
    uint64_t min = info[0].As<Napi::BigInt>().Uint64Value(&lossless);
    if (!lossless) {
        Napi::Error::New(env, "Loss of precision in min value").ThrowAsJavaScriptException();
        return env.Null();
    }
    uint64_t max = info[1].As<Napi::BigInt>().Uint64Value(&lossless);
    if (!lossless) {
        Napi::Error::New(env, "Loss of precision in max value").ThrowAsJavaScriptException();
        return env.Null();
    }

    uint64_t value = qrng_range64(ctx, min, max);
    return Napi::BigInt::New(env, value);
}

Napi::Value QuantumRNG::GetEntropyEstimate(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    double entropy = qrng_get_entropy_estimate(ctx);
    return Napi::Number::New(env, entropy);
}

Napi::Value QuantumRNG::Reseed(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    if (info.Length() < 1 || !info[0].IsBuffer()) {
        Napi::TypeError::New(env, "Seed buffer required").ThrowAsJavaScriptException();
        return env.Null();
    }

    Napi::Buffer<uint8_t> seedBuffer = info[0].As<Napi::Buffer<uint8_t>>();
    qrng_error err = qrng_reseed(ctx, seedBuffer.Data(), seedBuffer.Length());

    if (err != QRNG_SUCCESS) {
        Napi::Error::New(env, qrng_error_string(err)).ThrowAsJavaScriptException();
        return env.Null();
    }

    return env.Undefined();
}

Napi::Value QuantumRNG::EntangleStates(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    if (info.Length() < 2 || !info[0].IsBuffer() || !info[1].IsBuffer()) {
        Napi::TypeError::New(env, "Two state buffers required").ThrowAsJavaScriptException();
        return env.Null();
    }

    Napi::Buffer<uint8_t> state1 = info[0].As<Napi::Buffer<uint8_t>>();
    Napi::Buffer<uint8_t> state2 = info[1].As<Napi::Buffer<uint8_t>>();

    if (state1.Length() != state2.Length()) {
        Napi::Error::New(env, "State buffers must be the same length").ThrowAsJavaScriptException();
        return env.Null();
    }

    qrng_error err = qrng_entangle_states(ctx, state1.Data(), state2.Data(), state1.Length());
    if (err != QRNG_SUCCESS) {
        Napi::Error::New(env, qrng_error_string(err)).ThrowAsJavaScriptException();
        return env.Null();
    }

    return env.Undefined();
}

Napi::Value QuantumRNG::MeasureState(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    if (info.Length() < 1 || !info[0].IsBuffer()) {
        Napi::TypeError::New(env, "State buffer required").ThrowAsJavaScriptException();
        return env.Null();
    }

    Napi::Buffer<uint8_t> state = info[0].As<Napi::Buffer<uint8_t>>();
    qrng_error err = qrng_measure_state(ctx, state.Data(), state.Length());

    if (err != QRNG_SUCCESS) {
        Napi::Error::New(env, qrng_error_string(err)).ThrowAsJavaScriptException();
        return env.Null();
    }

    return env.Undefined();
}

Napi::Value QuantumRNG::GetVersion(const Napi::CallbackInfo& info) {
    return Napi::String::New(info.Env(), qrng_version());
}

Napi::Object Init(Napi::Env env, Napi::Object exports) {
    return QuantumRNG::Init(env, exports);
}

NODE_API_MODULE(quantum_rng, Init)
