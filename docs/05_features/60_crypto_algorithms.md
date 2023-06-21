The following new crypto APIs, along with integration tests, are added to taurus-cdt:

## ECDSA signature verification

### New APIs:

- `verify_ecdsa_sig`: return true if standard ECDSA signature verification succeeds, otherwise return false. There are two overloaded functions:
  - `bool verify_ecdsa_sig(const char* msg, uint32_t msg_len, const char* sig, uint32_t sig_len, const char* pubkey, uint32_t pubkey_len)`
  - `bool verify_ecdsa_sig(const std::string& msg, const std::string& sig, const std::string& pubkey)`

- `is_supported_ecdsa_pubkey`: return true if pubkey is in X.509 SubjectPublicKeyInfo format and PEM encoded. There are two overloaded functions:
  - `bool is_supported_ecdsa_pubkey(const char* pubkey, uint32_t pubkey_len)`
  - `bool is_supported_ecdsa_pubkey(const std::string& pubkey)`

### Example parameters:
  - msg: raw message string
    - e.g. "message to sign"
  - sig: ECDSA signature in ASN.1 DER format, base64 encoded string
    - e.g. "MEYCIQCi5byy/JAvLvFWjMP8ls7z0ttP8E9UApmw69OBzFWJ3gIhANFE2l3jO3L8c/kwEfuWMnh8q1BcrjYx3m368Xc/7QJU"
  - pubkey: ECDSA public key in X.509 SubjectPublicKeyInfo format, PEM encoded string. Note: newline char '\n' is needed in the string
    - e.g.
"-----BEGIN PUBLIC KEY-----\n"
"MFkwEwYHKoZIzj0CAQYIKoZIzj0DAQcDQgAEzjca5ANoUF+XT+4gIZj2/X3V2UuT\n"
"E9MTw3sQVcJzjyC/p7KeaXommTC/7n501p4Gd1TiTiH+YM6fw/YYJUPSPg==\n"
"-----END PUBLIC KEY-----"

### Protocol feature
`verify_ecdsa_sig` which has
- description digest `d05fe0811d2bce3ff737f351aa2ddd3ad2411c4c40f90b03a67577dbd9347ecf` and
- feature digest `fe3fb515e05e40f47d7a2058836200dd4b478241bdcb36bf175f9a40a056b5e3`


## RSA signature verification

### New APIs:

- `verify_rsa_sha256_sig`: return true if standard RSA signature verification succeeds, otherwise return false. There are two overloaded functions:
  - `bool verify_rsa_sha256_sig(const char* msg, uint32_t msglen, const char* sig, uint32_t siglen, const char* exp, uint32_t explen, const char* mod, uint32_t modlen)`
  - `bool verify_rsa_sha256_sig(const std::string& msg, const std::string& sig, const std::string& exp, const std::string& mod)`

### Example parameters:
  - msg: raw message string
    - e.g. "message to sign"
  - sig: RSA SHA-256 signature, in hex string format
    - e.g. "62afd13281f01a5b5a9710c274cc9b58f9a92f8575a7b2099cd5038173d838be"
           "b77cd2912e96166a724b7fb8391c96a67e18208a52047755a5af2d01101966e5"
           "40a3e9e075675b69faa177d3401673834459ee977a8b5c11db4351e61286207d"
           "25a5194bdbd66cb57ee716dda4d342f7eec09303500a2f0cf3c8141d46fa821f"
  - exp: RSA public key exponent, in hex string format
    - e.g."3"
  - mod: RSA public key modulus, in hex string format
    - e.g. "dff568a53cafdba7b1cd654fef54ed61649cdd6cb29fa743e35c73fcba7ef9c2"
           "b25a3b91e295abcea9aa5af0625f8b06428ec3140f2dd3c60c7dbb698cb3dbf6"
           "c64b1160daec4eb7d6deca1dfc45b83d5f30e5398f6f737ee394d57c8d2bf412"
           "f056c2e8a54d9bf554149c0da31346e31f23ffb516b1f9797d650169199b7add"
### Protocol feature
`verify_rsa_sha256_sig`, which has
- description digest `46c74376222421ef2827512e88ed7ccfa59e0fba00c9b0b7b5cf35315d079411`
- feature digest `00bca72bd868bc602036e6dea1ede57665b57203e3daaf18e6992e77d0d0341c`
