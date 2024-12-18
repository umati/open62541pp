#include <doctest/doctest.h>

#include "open62541pp/config.hpp"
#include "open62541pp/ua/types.hpp"

using namespace opcua;

TEST_CASE("EnumValueType") {
    const EnumValueType enumValueType(1, {"", "Name"}, {"", "Description"});
    CHECK(enumValueType.getValue() == 1);
    CHECK(enumValueType.getDisplayName() == LocalizedText("", "Name"));
    CHECK(enumValueType.getDescription() == LocalizedText("", "Description"));
}

TEST_CASE("RequestHeader") {
    const auto now = DateTime::now();
    const RequestHeader header({1, 1000}, now, 1, 2, "auditEntryId", 10, {});
    CHECK(header.getAuthenticationToken() == NodeId(1, 1000));
    CHECK(header.getTimestamp() == now);
    CHECK(header.getRequestHandle() == 1);
    CHECK(header.getReturnDiagnostics() == 2);
    CHECK(header.getAuditEntryId() == String("auditEntryId"));
    CHECK(header.getAdditionalHeader().isEmpty());
}

TEST_CASE("UserTokenPolicy") {
    UserTokenPolicy token(
        "policyId",
        UserTokenType::Username,
        "issuedTokenType",
        "issuerEndpointUrl",
        "securityPolicyUri"
    );
    CHECK(token.getPolicyId() == String("policyId"));
    CHECK(token.getTokenType() == UserTokenType::Username);
    CHECK(token.getIssuedTokenType() == String("issuedTokenType"));
    CHECK(token.getIssuerEndpointUrl() == String("issuerEndpointUrl"));
    CHECK(token.getSecurityPolicyUri() == String("securityPolicyUri"));
}

TEST_CASE("NodeAttributes") {
    // getters/setters are generated by specialized macros
    // just test the macros with VariableAttributes here
    VariableAttributes attr;
    CHECK(attr.getSpecifiedAttributes() == UA_NODEATTRIBUTESMASK_NONE);

    SUBCASE("Primitive type") {
        attr.setWriteMask(WriteMask::DataType);
        CHECK(attr.getWriteMask() == UA_WRITEMASK_DATATYPE);
        CHECK(attr.getSpecifiedAttributes() == UA_NODEATTRIBUTESMASK_WRITEMASK);
    }

    SUBCASE("Cast type") {
        attr.setValueRank(ValueRank::TwoDimensions);
        CHECK(attr.getValueRank() == ValueRank::TwoDimensions);
        CHECK(attr.getSpecifiedAttributes() == UA_NODEATTRIBUTESMASK_VALUERANK);
    }

    SUBCASE("Wrapper type") {
        attr.setDisplayName({"", "Name"});
        CHECK(attr.getDisplayName() == LocalizedText{"", "Name"});
        CHECK(attr.getSpecifiedAttributes() == UA_NODEATTRIBUTESMASK_DISPLAYNAME);
    }

    SUBCASE("Array type") {
        CHECK(attr.getArrayDimensions().empty());
        // assign twice to check deallocation
        attr.setArrayDimensions({1});
        attr.setArrayDimensions({1, 2});
        CHECK(attr.getArrayDimensions() == Span<const uint32_t>{1, 2});
        CHECK(attr.getSpecifiedAttributes() == UA_NODEATTRIBUTESMASK_ARRAYDIMENSIONS);
    }
}

TEST_CASE("NodeAttributes fluent interface") {
    const auto attr = NodeAttributes{}.setDisplayName({"", "displayName"}).setWriteMask(0xFFFFFFFF);
    CHECK(attr.getDisplayName() == LocalizedText("", "displayName"));
    CHECK(attr.getWriteMask() == 0xFFFFFFFF);
}

TEST_CASE_TEMPLATE("NodeAttributes setDataType", T, VariableAttributes, VariableTypeAttributes) {
    CHECK(T{}.setDataType(DataTypeId::Boolean).getDataType() == NodeId(DataTypeId::Boolean));
    CHECK(T{}.template setDataType<bool>().getDataType() == NodeId(DataTypeId::Boolean));
}

TEST_CASE("UserNameIdentityToken") {
    const UserNameIdentityToken token("userName", "password", "encryptionAlgorithm");
    CHECK(token.getPolicyId().empty());
    CHECK(token.getUserName() == String("userName"));
    CHECK(token.getPassword() == ByteString("password"));
    CHECK(token.getEncryptionAlgorithm() == String("encryptionAlgorithm"));
}

TEST_CASE("X509IdentityToken") {
    const X509IdentityToken token(ByteString("certificateData"));
    CHECK(token.getPolicyId().empty());
    CHECK(token.getCertificateData() == ByteString("certificateData"));
}

TEST_CASE("IssuedIdentityToken") {
    const IssuedIdentityToken token(ByteString("tokenData"), "encryptionAlgorithm");
    CHECK(token.getPolicyId().empty());
    CHECK(token.getTokenData() == ByteString("tokenData"));
    CHECK(token.getEncryptionAlgorithm() == String("encryptionAlgorithm"));
}

TEST_CASE("AddNodesItem / AddNodesRequest") {
    const AddNodesItem item(
        ExpandedNodeId({1, 1000}),
        {1, 1001},
        ExpandedNodeId({1, 1002}),
        {1, "item"},
        NodeClass::Object,
        ExtensionObject::fromDecodedCopy(ObjectAttributes{}),
        ExpandedNodeId({1, 1003})
    );
    CHECK(item.getParentNodeId().nodeId() == NodeId(1, 1000));
    CHECK(item.getReferenceTypeId() == NodeId(1, 1001));
    CHECK(item.getRequestedNewNodeId().nodeId() == NodeId(1, 1002));
    CHECK(item.getBrowseName() == QualifiedName(1, "item"));
    CHECK(item.getNodeClass() == NodeClass::Object);
    CHECK(item.getNodeAttributes().getDecodedDataType() == &UA_TYPES[UA_TYPES_OBJECTATTRIBUTES]);
    CHECK(item.getTypeDefinition().nodeId() == NodeId(1, 1003));

    const AddNodesRequest request({}, {item});
    CHECK_NOTHROW(request.getRequestHeader());
    CHECK(request.getNodesToAdd().size() == 1);
}

TEST_CASE("AddReferencesItem / AddReferencesRequest") {
    const AddReferencesItem item(
        {1, 1000}, {1, 1001}, true, {}, ExpandedNodeId({1, 1002}), NodeClass::Object
    );
    CHECK(item.getSourceNodeId() == NodeId(1, 1000));
    CHECK(item.getReferenceTypeId() == NodeId(1, 1001));
    CHECK(item.getIsForward() == true);
    CHECK(item.getTargetServerUri().empty());
    CHECK(item.getTargetNodeId().nodeId() == NodeId(1, 1002));
    CHECK(item.getTargetNodeClass() == NodeClass::Object);

    const AddReferencesRequest request({}, {item});
    CHECK_NOTHROW(request.getRequestHeader());
    CHECK(request.getReferencesToAdd().size() == 1);
}

TEST_CASE("DeleteNodesItem / DeleteNodesRequest") {
    const DeleteNodesItem item({1, 1000}, true);
    CHECK(item.getNodeId() == NodeId(1, 1000));
    CHECK(item.getDeleteTargetReferences() == true);

    const DeleteNodesRequest request({}, {item});
    CHECK_NOTHROW(request.getRequestHeader());
    CHECK(request.getNodesToDelete().size() == 1);
}

TEST_CASE("DeleteReferencesItem / DeleteReferencesRequest") {
    const DeleteReferencesItem item({1, 1000}, {1, 1001}, true, ExpandedNodeId({1, 1002}), true);
    CHECK(item.getSourceNodeId() == NodeId(1, 1000));
    CHECK(item.getReferenceTypeId() == NodeId(1, 1001));
    CHECK(item.getIsForward() == true);
    CHECK(item.getTargetNodeId().nodeId() == NodeId(1, 1002));
    CHECK(item.getDeleteBidirectional() == true);

    const DeleteReferencesRequest request({}, {item});
    CHECK_NOTHROW(request.getRequestHeader());
    CHECK(request.getReferencesToDelete().size() == 1);
}

TEST_CASE("ViewDescription") {
    const ViewDescription vd({1, 1000}, 12345U, 2U);
    CHECK(vd.getViewId() == NodeId(1, 1000));
    CHECK(vd.getTimestamp() == 12345U);
    CHECK(vd.getViewVersion() == 2U);
}

TEST_CASE("BrowseDescription") {
    const BrowseDescription bd(NodeId(1, 1000), BrowseDirection::Forward);
    CHECK(bd.getNodeId() == NodeId(1, 1000));
    CHECK(bd.getBrowseDirection() == BrowseDirection::Forward);
    CHECK(bd.getReferenceTypeId() == NodeId(0, UA_NS0ID_REFERENCES));
    CHECK(bd.getIncludeSubtypes() == true);
    CHECK(bd.getNodeClassMask() == UA_NODECLASS_UNSPECIFIED);
    CHECK(bd.getResultMask() == UA_BROWSERESULTMASK_ALL);
}

TEST_CASE("RelativePathElement") {
    const RelativePathElement rpe(ReferenceTypeId::HasComponent, false, false, {0, "test"});
    CHECK(rpe.getReferenceTypeId() == NodeId{0, UA_NS0ID_HASCOMPONENT});
    CHECK(rpe.getIsInverse() == false);
    CHECK(rpe.getIncludeSubtypes() == false);
    CHECK(rpe.getTargetName() == QualifiedName(0, "test"));
}

TEST_CASE("RelativePath") {
    const RelativePath rp{
        {ReferenceTypeId::HasComponent, false, false, {0, "child1"}},
        {ReferenceTypeId::HasComponent, false, false, {0, "child2"}},
    };
    const auto elements = rp.getElements();
    CHECK(elements.size() == 2);
    CHECK(elements[0].getTargetName() == QualifiedName(0, "child1"));
    CHECK(elements[1].getTargetName() == QualifiedName(0, "child2"));
}

TEST_CASE("BrowsePath") {
    const BrowsePath bp(
        ObjectId::ObjectsFolder, {{ReferenceTypeId::HasComponent, false, false, {0, "child"}}}
    );
    CHECK(bp.getStartingNode() == NodeId(0, UA_NS0ID_OBJECTSFOLDER));
    CHECK(bp.getRelativePath().getElements().size() == 1);
}

TEST_CASE("BrowseRequest") {
    const BrowseRequest request({}, {{1, 1000}, {}, 1}, 11U, {});
    CHECK_NOTHROW(request.getRequestHeader());
    CHECK(request.getView().getViewId() == NodeId(1, 1000));
    CHECK(request.getView().getViewVersion() == 1);
    CHECK(request.getRequestedMaxReferencesPerNode() == 11U);
    CHECK(request.getNodesToBrowse().empty());
}

TEST_CASE("BrowseNextRequest") {
    const BrowseNextRequest request({}, true, {ByteString("123")});
    CHECK_NOTHROW(request.getRequestHeader());
    CHECK(request.getReleaseContinuationPoints() == true);
    CHECK(request.getContinuationPoints().size() == 1);
    CHECK(request.getContinuationPoints()[0] == ByteString("123"));
}

TEST_CASE("TranslateBrowsePathsToNodeIdsRequest") {
    const TranslateBrowsePathsToNodeIdsRequest request({}, {});
    CHECK_NOTHROW(request.getRequestHeader());
    CHECK(request.getBrowsePaths().empty());
}

TEST_CASE("RegisterNodesRequest") {
    const RegisterNodesRequest request({}, {{1, 1000}});
    CHECK_NOTHROW(request.getRequestHeader());
    CHECK(request.getNodesToRegister().size() == 1);
    CHECK(request.getNodesToRegister()[0] == NodeId(1, 1000));
}

TEST_CASE("UnregisterNodesRequest") {
    const UnregisterNodesRequest request({}, {{1, 1000}});
    CHECK_NOTHROW(request.getRequestHeader());
    CHECK(request.getNodesToUnregister().size() == 1);
    CHECK(request.getNodesToUnregister()[0] == NodeId(1, 1000));
}

TEST_CASE("ReadValueId") {
    const ReadValueId rvid(NodeId(1, 1000), AttributeId::Value);
    CHECK(rvid.getNodeId() == NodeId(1, 1000));
    CHECK(rvid.getAttributeId() == AttributeId::Value);
    CHECK(rvid.getIndexRange().empty());
    CHECK(rvid.getDataEncoding() == QualifiedName());
}

TEST_CASE("ReadRequest") {
    const ReadRequest request(
        {},
        111.11,
        TimestampsToReturn::Both,
        {
            {{1, 1000}, AttributeId::Value},
        }
    );
    CHECK_NOTHROW(request.getRequestHeader());
    CHECK(request.getMaxAge() == 111.11);
    CHECK(request.getTimestampsToReturn() == TimestampsToReturn::Both);
    CHECK(request.getNodesToRead().size() == 1);
    CHECK(request.getNodesToRead()[0].getNodeId() == NodeId(1, 1000));
    CHECK(request.getNodesToRead()[0].getAttributeId() == AttributeId::Value);
}

TEST_CASE("WriteValue") {
    const WriteValue wv({1, 1000}, AttributeId::Value, {}, DataValue::fromScalar(11.11));
    CHECK(wv.getNodeId() == NodeId(1, 1000));
    CHECK(wv.getAttributeId() == AttributeId::Value);
    CHECK(wv.getIndexRange().empty());
    CHECK(wv.getValue().value().getScalar<double>() == 11.11);
}

TEST_CASE("WriteRequest") {
    const WriteRequest request(
        {},
        {
            {{1, 1000}, AttributeId::Value, {}, DataValue::fromScalar(11.11)},
        }
    );
    CHECK_NOTHROW(request.getRequestHeader());
    CHECK(request.getNodesToWrite().size() == 1);
    CHECK(request.getNodesToWrite()[0].getNodeId() == NodeId(1, 1000));
    CHECK(request.getNodesToWrite()[0].getAttributeId() == AttributeId::Value);
    CHECK(request.getNodesToWrite()[0].getValue().value().getScalar<double>() == 11.11);
}

TEST_CASE("WriteResponse") {
    const WriteResponse response;
    CHECK_NOTHROW(response.getResponseHeader());
    CHECK(response.getResults().empty());
    CHECK(response.getDiagnosticInfos().empty());
}

TEST_CASE("BuildInfo") {
    const BuildInfo buildInfo(
        "productUri",
        "manufacturerName",
        "productName",
        "softwareVersion",
        "buildNumber",
        DateTime(1234)
    );
    CHECK(buildInfo.getProductUri() == "productUri");
    CHECK(buildInfo.getManufacturerName() == "manufacturerName");
    CHECK(buildInfo.getProductName() == "productName");
    CHECK(buildInfo.getSoftwareVersion() == "softwareVersion");
    CHECK(buildInfo.getBuildNumber() == "buildNumber");
    CHECK(buildInfo.getBuildDate().get() == 1234);
}

#ifdef UA_ENABLE_METHODCALLS

TEST_CASE("Argument") {
    const Argument argument(
        "name", {"", "description"}, DataTypeId::Int32, ValueRank::TwoDimensions, {2, 3}
    );
    CHECK(argument.getName() == String("name"));
    CHECK(argument.getDescription() == LocalizedText("", "description"));
    CHECK(argument.getDataType() == NodeId(DataTypeId::Int32));
    CHECK(argument.getValueRank() == ValueRank::TwoDimensions);
    CHECK(argument.getArrayDimensions().size() == 2);
    CHECK(argument.getArrayDimensions()[0] == 2);
    CHECK(argument.getArrayDimensions()[1] == 3);
}

TEST_CASE("CallMethodRequest / CallRequest") {
    const CallMethodRequest item({1, 1000}, {1, 1001}, {Variant::fromScalar(11)});
    const CallRequest request({}, {item});
    CHECK(request.getMethodsToCall().size() == 1);
    CHECK(request.getMethodsToCall()[0].getObjectId() == NodeId(1, 1000));
    CHECK(request.getMethodsToCall()[0].getMethodId() == NodeId(1, 1001));
    CHECK(request.getMethodsToCall()[0].getInputArguments().size() == 1);
}

#endif

#ifdef UA_ENABLE_SUBSCRIPTIONS

TEST_CASE("ElementOperand") {
    CHECK(ElementOperand(11).getIndex() == 11);
}

TEST_CASE("LiteralOperand") {
    CHECK(LiteralOperand(Variant::fromScalar(11)).getValue().getScalar<int>() == 11);
    CHECK(LiteralOperand(11).getValue().getScalar<int>() == 11);
}

TEST_CASE("AttributeOperand") {
    const AttributeOperand operand(
        ObjectTypeId::BaseEventType,
        "alias",
        {
            {ReferenceTypeId::HasComponent, false, false, {0, "child1"}},
            {ReferenceTypeId::HasComponent, false, false, {0, "child2"}},
        },
        AttributeId::Value,
        {}
    );
    CHECK(operand.getNodeId() == NodeId(ObjectTypeId::BaseEventType));
    CHECK(operand.getAlias() == String("alias"));
    CHECK(operand.getBrowsePath().getElements().size() == 2);
    CHECK(operand.getAttributeId() == AttributeId::Value);
    CHECK(operand.getIndexRange().empty());
}

TEST_CASE("SimpleAttributeOperand") {
    const SimpleAttributeOperand operand(
        ObjectTypeId::BaseEventType, {{0, "child1"}, {0, "child2"}}, AttributeId::Value, {}
    );
    CHECK(operand.getTypeDefinitionId() == NodeId(ObjectTypeId::BaseEventType));
    CHECK(operand.getBrowsePath().size() == 2);
    CHECK(operand.getAttributeId() == AttributeId::Value);
    CHECK(operand.getIndexRange().empty());
}

TEST_CASE("ContentFilter(Element)") {
    const ContentFilter contentFilter{
        {FilterOperator::And, {ElementOperand(1), ElementOperand(2)}},
        {FilterOperator::OfType, {LiteralOperand(NodeId(ObjectTypeId::BaseEventType))}},
        {FilterOperator::Equals, {LiteralOperand(99), LiteralOperand(99)}},
    };

    auto elements = contentFilter.getElements();
    CHECK(elements.size() == 3);
    CHECK(elements[0].getFilterOperator() == FilterOperator::And);
    CHECK(elements[0].getFilterOperands().size() == 2);
    CHECK(elements[0].getFilterOperands()[0].getDecodedData<ElementOperand>()->getIndex() == 1);
    CHECK(elements[0].getFilterOperands()[1].getDecodedData<ElementOperand>()->getIndex() == 2);
    CHECK(elements[1].getFilterOperator() == FilterOperator::OfType);
    CHECK(elements[1].getFilterOperands().size() == 1);
    CHECK(elements[1].getFilterOperands()[0].getDecodedData<LiteralOperand>() != nullptr);
    CHECK(elements[2].getFilterOperator() == FilterOperator::Equals);
    CHECK(elements[2].getFilterOperands().size() == 2);
    CHECK(elements[2].getFilterOperands()[0].getDecodedData<LiteralOperand>() != nullptr);
    CHECK(elements[2].getFilterOperands()[1].getDecodedData<LiteralOperand>() != nullptr);
}

TEST_CASE("ContentFilter(Element) operators") {
    const ContentFilterElement filterElement(
        FilterOperator::GreaterThan,
        {
            SimpleAttributeOperand(
                ObjectTypeId::BaseEventType, {{0, "Severity"}}, AttributeId::Value
            ),
            LiteralOperand(200),
        }
    );

    const ContentFilter filter{
        {FilterOperator::And, {ElementOperand(1), ElementOperand(2)}},
        {FilterOperator::OfType, {LiteralOperand(NodeId(ObjectTypeId::BaseEventType))}},
        {FilterOperator::Equals, {LiteralOperand(99), LiteralOperand(99)}},
    };

    auto elementOperandIndex = [](const ExtensionObject& operand) {
        return operand.getDecodedData<ElementOperand>()->getIndex();
    };

    auto firstOperator = [](const ContentFilter& contentFilter) {
        return contentFilter.getElements()[0].getFilterOperator();
    };

    SUBCASE("Not") {
        const ContentFilter filterElementNot = !filterElement;
        CHECK(filterElementNot.getElements().size() == 2);
        CHECK(firstOperator(filterElementNot) == FilterOperator::Not);
        CHECK(elementOperandIndex(filterElementNot.getElements()[0].getFilterOperands()[0]) == 1);
        CHECK(filterElementNot.getElements()[1].getFilterOperator() == FilterOperator::GreaterThan);

        const ContentFilter filterNot = !filter;
        CHECK(filterNot.getElements().size() == 4);
        CHECK(firstOperator(filterNot) == FilterOperator::Not);
        CHECK(elementOperandIndex(filterNot.getElements()[0].getFilterOperands()[0]) == 1);
        CHECK(elementOperandIndex(filterNot.getElements()[1].getFilterOperands()[0]) == 2);
        CHECK(elementOperandIndex(filterNot.getElements()[1].getFilterOperands()[1]) == 3);
    }

    SUBCASE("And") {
        CHECK((filterElement && filterElement).getElements().size() == 3);
        CHECK((filterElement && filter).getElements().size() == 5);
        CHECK((filter && filterElement).getElements().size() == 5);
        CHECK((filter && filter).getElements().size() == 7);

        CHECK(firstOperator(filterElement && filterElement) == FilterOperator::And);
        CHECK(firstOperator(filterElement && filter) == FilterOperator::And);
        CHECK(firstOperator(filter && filterElement) == FilterOperator::And);
        CHECK(firstOperator(filter && filter) == FilterOperator::And);

        SUBCASE("Increment operand indexes") {
            const ContentFilter filterAdd = filter && filter;
            CHECK(filterAdd.getElements().size() == 7);
            // and
            CHECK(elementOperandIndex(filterAdd.getElements()[0].getFilterOperands()[0]) == 1);
            CHECK(elementOperandIndex(filterAdd.getElements()[0].getFilterOperands()[1]) == 4);
            // lhs and
            CHECK(elementOperandIndex(filterAdd.getElements()[1].getFilterOperands()[0]) == 2);
            CHECK(elementOperandIndex(filterAdd.getElements()[1].getFilterOperands()[1]) == 3);
            // rhs and
            CHECK(elementOperandIndex(filterAdd.getElements()[4].getFilterOperands()[0]) == 5);
            CHECK(elementOperandIndex(filterAdd.getElements()[4].getFilterOperands()[1]) == 6);
        }
    }

    SUBCASE("Or") {
        CHECK((filterElement || filterElement).getElements().size() == 3);
        CHECK((filterElement || filter).getElements().size() == 5);
        CHECK((filter || filterElement).getElements().size() == 5);
        CHECK((filter || filter).getElements().size() == 7);

        CHECK(firstOperator(filterElement || filterElement) == FilterOperator::Or);
        CHECK(firstOperator(filterElement || filter) == FilterOperator::Or);
        CHECK(firstOperator(filter || filterElement) == FilterOperator::Or);
        CHECK(firstOperator(filter || filter) == FilterOperator::Or);
    }
}

TEST_CASE("DataChangeFilter") {
    const DataChangeFilter dataChangeFilter(
        DataChangeTrigger::StatusValue, DeadbandType::Percent, 11.11
    );

    CHECK(dataChangeFilter.getTrigger() == DataChangeTrigger::StatusValue);
    CHECK(dataChangeFilter.getDeadbandType() == DeadbandType::Percent);
    CHECK(dataChangeFilter.getDeadbandValue() == 11.11);
}

TEST_CASE("EventFilter") {
    const EventFilter eventFilter(
        {
            {{}, {{0, "Time"}}, AttributeId::Value},
            {{}, {{0, "Severity"}}, AttributeId::Value},
            {{}, {{0, "Message"}}, AttributeId::Value},
        },
        {
            {FilterOperator::OfType, {LiteralOperand(NodeId(ObjectTypeId::BaseEventType))}},
        }
    );

    CHECK(eventFilter.getSelectClauses().size() == 3);
    CHECK(eventFilter.getWhereClause().getElements().size() == 1);
}

TEST_CASE("AggregateFilter") {
    const DateTime startTime = DateTime::now();
    AggregateConfiguration aggregateConfiguration{};
    aggregateConfiguration.useSlopedExtrapolation = true;

    const AggregateFilter aggregateFilter(
        startTime, ObjectId::AggregateFunction_Average, 11.11, aggregateConfiguration
    );

    CHECK(aggregateFilter.getStartTime() == startTime);
    CHECK(aggregateFilter.getAggregateType() == NodeId(ObjectId::AggregateFunction_Average));
    CHECK(aggregateFilter.getProcessingInterval() == 11.11);
    CHECK(aggregateFilter.getAggregateConfiguration().useSlopedExtrapolation == true);
}

TEST_CASE("MonitoringParameters") {
    const MonitoringParameters params(11.11, {}, 10, false);
    CHECK(params.getSamplingInterval() == 11.11);
    CHECK(params.getFilter().isEmpty());
    CHECK(params.getQueueSize() == 10);
    CHECK(params.getDiscardOldest() == false);
}

TEST_CASE("MonitoredItemCreateRequest / CreateMonitoredItemsRequest") {
    const MonitoredItemCreateRequest item({{1, 1000}, AttributeId::Value});
    CHECK(item.getItemToMonitor().getNodeId() == NodeId(1, 1000));
    CHECK(item.getItemToMonitor().getAttributeId() == AttributeId::Value);
    CHECK(item.getMonitoringMode() == MonitoringMode::Reporting);

    const CreateMonitoredItemsRequest request({}, 1U, TimestampsToReturn::Both, {item});
    CHECK(request.getSubscriptionId() == 1U);
    CHECK(request.getTimestampsToReturn() == TimestampsToReturn::Both);
    CHECK(request.getItemsToCreate().size() == 1);
}

TEST_CASE("MonitoredItemModifyRequest / ModifyMonitoredItemsRequest") {
    const MonitoredItemModifyRequest item(1U, MonitoringParameters(11.11));
    CHECK(item.getMonitoredItemId() == 1U);
    CHECK(item.getRequestedParameters().getSamplingInterval() == 11.11);

    const ModifyMonitoredItemsRequest request({}, 1U, TimestampsToReturn::Both, {item});
    CHECK(request.getSubscriptionId() == 1U);
    CHECK(request.getTimestampsToReturn() == TimestampsToReturn::Both);
    CHECK(request.getItemsToModify().size() == 1);
}

TEST_CASE("SetMonitoringModeRequest") {
    const SetMonitoringModeRequest request({}, 1U, MonitoringMode::Reporting, {0U, 1U});
    CHECK_NOTHROW(request.getRequestHeader());
    CHECK(request.getSubscriptionId() == 1U);
    CHECK(request.getMonitoringMode() == MonitoringMode::Reporting);
    CHECK(request.getMonitoredItemIds().size() == 2);
    CHECK(request.getMonitoredItemIds()[0] == 0U);
    CHECK(request.getMonitoredItemIds()[1] == 1U);
}

TEST_CASE("SetTriggeringRequest") {
    const SetTriggeringRequest request({}, 1U, 2U, {3U}, {4U, 5U});
    CHECK_NOTHROW(request.getRequestHeader());
    CHECK(request.getSubscriptionId() == 1U);
    CHECK(request.getTriggeringItemId() == 2U);
    CHECK(request.getLinksToAdd().size() == 1);
    CHECK(request.getLinksToAdd()[0] == 3U);
    CHECK(request.getLinksToRemove().size() == 2);
    CHECK(request.getLinksToRemove()[0] == 4U);
    CHECK(request.getLinksToRemove()[1] == 5U);
}

TEST_CASE("DeleteMonitoredItemsRequest") {
    const DeleteMonitoredItemsRequest request({}, 1U, {0U, 1U});
    CHECK_NOTHROW(request.getRequestHeader());
    CHECK(request.getSubscriptionId() == 1U);
    CHECK(request.getMonitoredItemIds().size() == 2);
    CHECK(request.getMonitoredItemIds()[0] == 0U);
    CHECK(request.getMonitoredItemIds()[1] == 1U);
}

TEST_CASE("CreateSubscriptionRequest") {
    const CreateSubscriptionRequest request({}, 11.11, 2, 3, 4, true, 5);
    CHECK_NOTHROW(request.getRequestHeader());
    CHECK(request.getRequestedPublishingInterval() == 11.11);
    CHECK(request.getRequestedLifetimeCount() == 2);
    CHECK(request.getRequestedMaxKeepAliveCount() == 3);
    CHECK(request.getMaxNotificationsPerPublish() == 4);
    CHECK(request.getPublishingEnabled() == true);
    CHECK(request.getPriority() == 5);
}

TEST_CASE("ModifySubscriptionRequest") {
    const ModifySubscriptionRequest request({}, 1, 11.11, 2, 3, 4, 5);
    CHECK_NOTHROW(request.getRequestHeader());
    CHECK(request.getSubscriptionId() == 1);
    CHECK(request.getRequestedPublishingInterval() == 11.11);
    CHECK(request.getRequestedLifetimeCount() == 2);
    CHECK(request.getRequestedMaxKeepAliveCount() == 3);
    CHECK(request.getMaxNotificationsPerPublish() == 4);
    CHECK(request.getPriority() == 5);
}

TEST_CASE("SetPublishingModeRequest") {
    const SetPublishingModeRequest request({}, true, {1, 2, 3});
    CHECK_NOTHROW(request.getRequestHeader());
    CHECK(request.getPublishingEnabled() == true);
    CHECK(request.getSubscriptionIds().size() == 3);
    CHECK(request.getSubscriptionIds()[0] == 1);
    CHECK(request.getSubscriptionIds()[1] == 2);
    CHECK(request.getSubscriptionIds()[2] == 3);
}

TEST_CASE("DeleteSubscriptionsRequest") {
    const DeleteSubscriptionsRequest request({}, {1, 2, 3});
    CHECK_NOTHROW(request.getRequestHeader());
    CHECK(request.getSubscriptionIds().size() == 3);
    CHECK(request.getSubscriptionIds()[0] == 1);
    CHECK(request.getSubscriptionIds()[1] == 2);
    CHECK(request.getSubscriptionIds()[2] == 3);
}

#endif

#if UAPP_HAS_DATAACCESS

TEST_CASE("Range") {
    const Range range(1.1, 2.2);
    CHECK(range.getLow() == 1.1);
    CHECK(range.getHigh() == 2.2);
}

TEST_CASE("EUInformation") {
    const EUInformation info("namespaceUri", 1, {"", "displayName"}, {"", "description"});
    CHECK(info.getNamespaceUri() == "namespaceUri");
    CHECK(info.getUnitId() == 1);
    CHECK(info.getDisplayName() == LocalizedText("", "displayName"));
    CHECK(info.getDescription() == LocalizedText("", "description"));
}

TEST_CASE("ComplexNumberType") {
    const ComplexNumberType complex(1.1f, 2.2f);
    CHECK(complex.getReal() == 1.1f);
    CHECK(complex.getImaginary() == 2.2f);
}

TEST_CASE("DoubleComplexNumberType") {
    const DoubleComplexNumberType complex(1.1, 2.2);
    CHECK(complex.getReal() == 1.1);
    CHECK(complex.getImaginary() == 2.2);
}

TEST_CASE("AxisInformation") {
    const AxisInformation axis(
        EUInformation("namespaceUri", 1, {}, {}),
        Range(1.1, 3.3),
        {"", "title"},
        AxisScaleEnumeration::Log,
        {1.1, 2.2, 3.3}
    );
    CHECK(axis.getEngineeringUnits().getNamespaceUri() == "namespaceUri");
    CHECK(axis.getEURange().getLow() == 1.1);
    CHECK(axis.getEURange().getHigh() == 3.3);
    CHECK(axis.getTitle() == LocalizedText("", "title"));
    CHECK(axis.getAxisScaleType() == AxisScaleEnumeration::Log);
    CHECK(axis.getAxisSteps().size() == 3);
    CHECK(axis.getAxisSteps()[0] == 1.1);
}

TEST_CASE("XVType") {
    const XVType xv(1.1, 2.2f);
    CHECK(xv.getX() == 1.1);
    CHECK(xv.getValue() == 2.2f);
}

#endif

#ifdef UA_ENABLE_TYPEDESCRIPTION

TEST_CASE("EnumField / EnumDefinition") {
    const EnumDefinition enumDefinition{{0, "Zero"}, {1, "One"}};
    CHECK(enumDefinition.getFields().size() == 2);
    CHECK(enumDefinition.getFields()[0].getValue() == 0);
    CHECK(enumDefinition.getFields()[0].getDisplayName() == LocalizedText("", "Zero"));
    CHECK(enumDefinition.getFields()[0].getDescription() == LocalizedText());
    CHECK(enumDefinition.getFields()[0].getName() == "Zero");
    CHECK(enumDefinition.getFields()[1].getValue() == 1);
    CHECK(enumDefinition.getFields()[1].getDisplayName() == LocalizedText("", "One"));
    CHECK(enumDefinition.getFields()[1].getDescription() == LocalizedText());
    CHECK(enumDefinition.getFields()[1].getName() == "One");
}

#endif
