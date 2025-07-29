#pragma once

namespace LLHttp{
    /// @brief For Http Response status codes
    enum class HttpStatus{
        Continue=100,
        SwitchingProtocols=101,
        Processing=102,
        EarlyHints=103,
        Ok=200,
        Created=201,
        Accepted=202,
        NonAuthoritativeInformation=203,
        NoContent=204,
        ResetContent=205,
        PartialContent=206,
        MultiStatus=207,
        AlreadyReported=208,
        IMUsed=226,
        MultipleChoices=300,
        MovedPermanently=301,
        Found=302,
        SeeOther=303,
        NotModified=304,
        UseProxy=305,
        Unused=306,
        TemporaryRedirect=307,
        PermanentRedirect=308,
        BadRequest=400,
        Unauthorized=401,
        PaymentRequired=402,
        Forbidden=403,
        NotFound=404,
        MethodNotAllowed=405,
        NotAcceptable=406,
        ProxyAuthenticationRequired=407,
        RequestTimeout=408,
        Conflict=409,
        Gone=410,
        LengthRequired=411,
        PreconditionFailed=412,
        ContentTooLarge=413,
        URITooLong=414,
        UnsupportedMediaType=415,
        RangeNotSatisfiable=416,
        ExpectationFailed=417,
        ImaTeapot=418,
        MisdirectedRequest=421,
        UnprocessableContent=422,
        Locked=423,
        FailedDependency=424,
        TooEarly=425,
        UpgradeRequired=426,
        PreconditionRequired=428,
        TooManyRequest=429,
        RequestHeaderFieldsTooLarge=431,
        UnavailableForLegalReasons=451,
        InternalServerError=500,
        NotImplemented=501,
        BadGateway=502,
        ServiceUnavailable=503,
        GatewayTimeout=504,
        HttpVersionNotSupported=505,
        VariantAlsoNegotiates=506,
        InsufficientStorage=507,
        LoopDetected=508,
        NotExtended=510,
        NetworkAuthenticationRequired=511

    };

    /// @brief For determining how a packet is parsed and sent
    enum class HttpVersion{
        Unsupported=0,
        HTTP0_9,
        HTTP1_0,
        HTTP1_1,
        HTTP2_0,
        HTTP3_0
    };

    //Only supported for now
    enum class HttpTransferEncoding{
        Unsupported=0,
        Identity,
        Chunked,
        Deflate,
    };

    enum class HttpContentEncoding{
        Unsupported = 0,
        Identity,
        GZip,
        Deflate,
        Brotli,
        Compress
    };


    /// @brief Error Codes for parsing request/responses
    enum class HttpParseErrorCode{
        None=0,
        NeedsMoreData=1,
        NoMoreBodies,
        InvalidHttpResponse,
        UnsupportedHttpProtocol,
        UnsupportedTransferEncoding,
        InvalidPath,
        InvalidHeaderName,
        InvalidHeaderSplit,
        InvalidHeaderValue,
        InvalidHeaderEnd,
        InvalidChunkSize,
        InvalidChunkEnd,
        FailedDecodeGZip,
        FailedEncodeGZip,
        InvalidState
    };

    /// @brief Error codes for decoding/encoding encrypted data. 
    enum class HttpEncodingErrorCode{
        None=0,
        NeedsMoreData,
        InitializationFailure,
        UnsupportedContentEncoding,
        BrotliDecoderNeedsMoreInput,
        BrotliDecoderNeedsMoreOutput,
        IllegalPercentEncodingDelimiter,
        IllegalPercentEncodingCharacter,
        IllegalPercentEncodingOpcode,
        InvalidHostname,
        InvalidPort,
        InvalidPath,
        InvalidMimeType,
        InvalidPercentEncodingCharacter,
        FailedDecodeGZip,
        FailedEncodeGZip,
        FailedDecodeBrotli,
        FailedEncoddeBrotli
    };

    /// @brief for parsing urls
    enum class URLProtocol{
        Http=0,
        Https=1
    };

    enum class URLParseError : uint8_t{
        None=0,
        InvalidHostname,
        NeedsMoreData,
        InvalidPort,
        InvalidPath
    };

    /// @brief For HttpRequest
    enum class HttpVerb{
        Unknown=0,
        Get,
        Post,
        Head,
        Delete,
        Put,
        Trace,
        Connect,
        Patch,
        Options,
    };
}