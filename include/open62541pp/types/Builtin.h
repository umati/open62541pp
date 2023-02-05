#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <string_view>

#include "open62541pp/TypeWrapper.h"
#include "open62541pp/open62541.h"

namespace opcua {

/**
 * UA_String wrapper class.
 * @ingroup TypeWrapper
 */
class String : public TypeWrapper<UA_String, UA_TYPES_STRING> {
public:
    using TypeWrapperBase::TypeWrapperBase;  // inherit contructors

    explicit String(std::string_view str);

    std::string get() const;
    std::string_view getView() const;
};

/**
 * UA_Guid wrapper class.
 * @ingroup TypeWrapper
 */
class Guid : public TypeWrapper<UA_Guid, UA_TYPES_GUID> {
public:
    using TypeWrapperBase::TypeWrapperBase;  // inherit contructors

    Guid(UA_UInt32 data1, UA_UInt16 data2, UA_UInt16 data3, std::array<UA_Byte, 8> data4);
};

/**
 * UA_ByteString wrapper class.
 * @ingroup TypeWrapper
 */
class ByteString : public TypeWrapper<UA_ByteString, UA_TYPES_BYTESTRING> {
public:
    // NOLINTNEXTLINE, false positive?
    using TypeWrapperBase::TypeWrapperBase;  // inherit contructors

    explicit ByteString(std::string_view str);

    std::string get() const;
    std::string_view getView() const;
};

/**
 * UA_XmlElement wrapper class.
 * @ingroup TypeWrapper
 */
class XmlElement : public TypeWrapper<UA_XmlElement, UA_TYPES_XMLELEMENT> {
public:
    // NOLINTNEXTLINE, false positive?
    using TypeWrapperBase::TypeWrapperBase;  // inherit contructors

    explicit XmlElement(std::string_view str);

    std::string get() const;
    std::string_view getView() const;
};

/**
 * UA_QualifiedName wrapper class.
 * @ingroup TypeWrapper
 */
class QualifiedName : public TypeWrapper<UA_QualifiedName, UA_TYPES_QUALIFIEDNAME> {
public:
    // NOLINTNEXTLINE, false positive?
    using TypeWrapperBase::TypeWrapperBase;  // inherit contructors

    QualifiedName(uint16_t namespaceIndex, std::string_view name);

    uint16_t getNamespaceIndex() const noexcept;
    std::string getName() const;
    std::string_view getNameView() const;
};

/**
 * UA_LocalizedText wrapper class.
 * @ingroup TypeWrapper
 */
class LocalizedText : public TypeWrapper<UA_LocalizedText, UA_TYPES_LOCALIZEDTEXT> {
public:
    // NOLINTNEXTLINE, false positive?
    using TypeWrapperBase::TypeWrapperBase;  // inherit contructors

    LocalizedText(std::string_view text, std::string_view locale);

    std::string getText() const;
    std::string_view getTextView() const;
    std::string getLocale() const;
    std::string_view getLocaleView() const;
};

}  // namespace opcua