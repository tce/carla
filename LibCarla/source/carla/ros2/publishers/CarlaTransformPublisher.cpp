#define _GLIBCXX_USE_CXX11_ABI 0

#include "CarlaTransformPublisher.h"

#include <string>
#include <chrono>

#include "carla/ros2/types/TFMessagePubSubTypes.h"
#include "carla/ros2/listeners/CarlaListener.h"

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/topic/Topic.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>

#include <fastdds/dds/domain/qos/DomainParticipantQos.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/publisher/qos/PublisherQos.hpp>
#include <fastdds/dds/topic/qos/TopicQos.hpp>

#include <fastrtps/attributes/ParticipantAttributes.h>
#include <fastrtps/qos/QosPolicies.h>
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>
#include <fastdds/dds/publisher/DataWriterListener.hpp>


namespace carla {
namespace ros2 {
  namespace efd = eprosima::fastdds::dds;
  using erc = eprosima::fastrtps::types::ReturnCode_t;

  struct CarlaTransformPublisherImpl {
    efd::DomainParticipant* _participant { nullptr };
    efd::Publisher* _publisher { nullptr };
    efd::Topic* _topic { nullptr };
    efd::DataWriter* _datawriter { nullptr };
    efd::TypeSupport _type { new tf2_msgs::msg::TFMessagePubSubType() };
    CarlaListener _listener {};
    tf2_msgs::msg::TFMessage _transform {};
  };

  bool CarlaTransformPublisher::Init() {
    if (_impl->_type == nullptr) {
        std::cerr << "Invalid TypeSupport" << std::endl;
        return false;
    }
    efd::DomainParticipantQos pqos = efd::PARTICIPANT_QOS_DEFAULT;
    pqos.name("CarlaPublisherParticipant");
    auto factory = efd::DomainParticipantFactory::get_instance();
    _impl->_participant = factory->create_participant(0, pqos);
    if (_impl->_participant == nullptr) {
        std::cerr << "Failed to create DomainParticipant" << std::endl;
        return false;
    }
    _impl->_type.register_type(_impl->_participant);
    efd::PublisherQos pubqos = efd::PUBLISHER_QOS_DEFAULT;
    _impl->_publisher = _impl->_participant->create_publisher(pubqos, nullptr);
    if (_impl->_publisher == nullptr) {
      std::cerr << "Failed to create Publisher" << std::endl;
      return false;
    }
    efd::TopicQos tqos = efd::TOPIC_QOS_DEFAULT;
    const std::string topic_name {"rt/carla/tf2"};
    _impl->_topic = _impl->_participant->create_topic(topic_name, _impl->_type->getName(), tqos);
    if (_impl->_topic == nullptr) {
        std::cerr << "Failed to create Topic" << std::endl;
        return false;
    }
    eprosima::fastrtps::ReliabilityQosPolicy rqos;
    rqos.kind = eprosima::fastrtps::BEST_EFFORT_RELIABILITY_QOS;
    efd::DataWriterQos wqos = efd::DATAWRITER_QOS_DEFAULT;
    wqos.endpoint().history_memory_policy = eprosima::fastrtps::rtps::PREALLOCATED_WITH_REALLOC_MEMORY_MODE;
    efd::DataWriterListener* listener = (efd::DataWriterListener*)_impl->_listener._impl.get();
    _impl->_datawriter = _impl->_publisher->create_datawriter(_impl->_topic, wqos, listener);
    if (_impl->_datawriter == nullptr) {
        std::cerr << "Failed to create DataWriter" << std::endl;
        return false;
    }
    return true;
  }

  bool CarlaTransformPublisher::Publish() {
    eprosima::fastrtps::rtps::InstanceHandle_t instance_handle;
    eprosima::fastrtps::types::ReturnCode_t rcode = _impl->_datawriter->write(&_impl->_transform, instance_handle);
    if (rcode == erc::ReturnCodeValue::RETCODE_OK) {
        return true;
    }
    if (rcode == erc::ReturnCodeValue::RETCODE_ERROR) {
        std::cerr << "RETCODE_ERROR" << std::endl;
        return false;
    }
    if (rcode == erc::ReturnCodeValue::RETCODE_UNSUPPORTED) {
        std::cerr << "RETCODE_UNSUPPORTED" << std::endl;
        return false;
    }
    if (rcode == erc::ReturnCodeValue::RETCODE_BAD_PARAMETER) {
        std::cerr << "RETCODE_BAD_PARAMETER" << std::endl;
        return false;
    }
    if (rcode == erc::ReturnCodeValue::RETCODE_PRECONDITION_NOT_MET) {
        std::cerr << "RETCODE_PRECONDITION_NOT_MET" << std::endl;
        return false;
    }
    if (rcode == erc::ReturnCodeValue::RETCODE_OUT_OF_RESOURCES) {
        std::cerr << "RETCODE_OUT_OF_RESOURCES" << std::endl;
        return false;
    }
    if (rcode == erc::ReturnCodeValue::RETCODE_NOT_ENABLED) {
        std::cerr << "RETCODE_NOT_ENABLED" << std::endl;
        return false;
    }
    if (rcode == erc::ReturnCodeValue::RETCODE_IMMUTABLE_POLICY) {
        std::cerr << "RETCODE_IMMUTABLE_POLICY" << std::endl;
        return false;
    }
    if (rcode == erc::ReturnCodeValue::RETCODE_INCONSISTENT_POLICY) {
        std::cerr << "RETCODE_INCONSISTENT_POLICY" << std::endl;
        return false;
    }
    if (rcode == erc::ReturnCodeValue::RETCODE_ALREADY_DELETED) {
        std::cerr << "RETCODE_ALREADY_DELETED" << std::endl;
        return false;
    }
    if (rcode == erc::ReturnCodeValue::RETCODE_TIMEOUT) {
        std::cerr << "RETCODE_TIMEOUT" << std::endl;
        return false;
    }
    if (rcode == erc::ReturnCodeValue::RETCODE_NO_DATA) {
        std::cerr << "RETCODE_NO_DATA" << std::endl;
        return false;
    }
    if (rcode == erc::ReturnCodeValue::RETCODE_ILLEGAL_OPERATION) {
        std::cerr << "RETCODE_ILLEGAL_OPERATION" << std::endl;
        return false;
    }
    if (rcode == erc::ReturnCodeValue::RETCODE_NOT_ALLOWED_BY_SECURITY) {
        std::cerr << "RETCODE_NOT_ALLOWED_BY_SECURITY" << std::endl;
        return false;
    }
    std::cout << "UNKNOWN" << std::endl;    
    return false;
  }

  void CarlaTransformPublisher::SetData(const float* translation, const float* rotation, const char* frame_id) {
    geometry_msgs::msg::Vector3 vec_translation;
    geometry_msgs::msg::Quaternion vec_rotation;
    float tx = *translation++;
    float ty = *translation++;
    float tz = *translation++;
    vec_translation.x(tx);
    vec_translation.y(ty);
    vec_translation.z(tz);
    float rx = *rotation++;
    float ry = *rotation++;
    float rz = *rotation++;
    float rw = *rotation++;
    vec_rotation.x(rx);
    vec_rotation.y(ry);
    vec_rotation.z(rz);
    vec_rotation.w(rw);

    auto epoch = std::chrono::system_clock::now().time_since_epoch();
    const auto secons = std::chrono::duration_cast<std::chrono::seconds>(epoch);
    epoch -= secons;
    const auto secs = static_cast<int32_t>(secons.count());
    const auto nanoseconds = static_cast<uint32_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(epoch).count());

    builtin_interfaces::msg::Time time;
    time.sec(secs);
    time.nanosec(nanoseconds);

    std_msgs::msg::Header header;
    header.stamp(std::move(time));
    header.frame_id(frame_id);

    geometry_msgs::msg::Transform t;
    t.rotation(vec_rotation);
    t.translation(vec_translation);
    geometry_msgs::msg::TransformStamped ts;
    ts.transform(t);
    _impl->_transform.transforms({ts});
  }

  CarlaTransformPublisher::CarlaTransformPublisher() :
  _impl(std::make_unique<CarlaTransformPublisherImpl>()) {}

  CarlaTransformPublisher::~CarlaTransformPublisher() {
      if (!_impl)
          return;

      if (_impl->_datawriter)
          _impl->_publisher->delete_datawriter(_impl->_datawriter);

      if (_impl->_publisher)
          _impl->_participant->delete_publisher(_impl->_publisher);

      if (_impl->_topic)
          _impl->_participant->delete_topic(_impl->_topic);
      
      if (_impl->_participant)
          efd::DomainParticipantFactory::get_instance()->delete_participant(_impl->_participant);
  }
}
}