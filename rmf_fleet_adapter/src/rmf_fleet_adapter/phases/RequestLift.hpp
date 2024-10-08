/*
 * Copyright (C) 2020 Open Source Robotics Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
*/

#ifndef SRC__RMF_FLEET_ADAPTER__PHASES__REQUESTLIFT_HPP
#define SRC__RMF_FLEET_ADAPTER__PHASES__REQUESTLIFT_HPP

#include "../LegacyTask.hpp"
#include "../agv/RobotContext.hpp"
#include "rmf_fleet_adapter/StandardNames.hpp"
#include "EndLiftSession.hpp"

namespace rmf_fleet_adapter {
namespace phases {

struct RequestLift
{
  enum class Located
  {
    Inside,
    Outside
  };

  struct Data
  {
    rmf_traffic::Time expected_finish;
    Located located;
    PlanIdPtr plan_id;
    std::optional<agv::Destination> localize_after = std::nullopt;
    std::shared_ptr<rmf_traffic::schedule::Itinerary> resume_itinerary =
      nullptr;
    std::optional<rmf_traffic::agv::Plan::Waypoint> hold_point = std::nullopt;

    std::optional<agv::RobotUpdateHandle::LiftDestination>
    final_lift_destination = std::nullopt;
  };

  class ActivePhase : public LegacyTask::ActivePhase,
    public std::enable_shared_from_this<ActivePhase>
  {
  public:

    static std::shared_ptr<ActivePhase> make(
      agv::RobotContextPtr context,
      std::string lift_name,
      std::string destination,
      Data data);

    const rxcpp::observable<LegacyTask::StatusMsg>& observe() const override;

    rmf_traffic::Duration estimate_remaining_time() const override;

    void emergency_alarm(bool on) override;

    void cancel() override;

    const std::string& description() const override;

  private:

    agv::RobotContextPtr _context;
    std::string _lift_name;
    std::string _destination;
    Data _data;
    rxcpp::subjects::behavior<bool> _cancelled =
      rxcpp::subjects::behavior<bool>(false);
    std::string _description;
    rxcpp::observable<LegacyTask::StatusMsg> _obs;
    rclcpp::TimerBase::SharedPtr _timer;
    std::shared_ptr<EndLiftSession::Active> _lift_end_phase;
    rmf_rxcpp::subscription_guard _reset_session_subscription;
    std::shared_ptr<void> _destination_handle;
    std::shared_ptr<std::string> _current_boarded_lift_level;
    bool _finished = false;

    struct WatchdogInfo
    {
      std::mutex mutex;
      std::optional<agv::RobotUpdateHandle::Unstable::Decision> decision;
    };

    std::shared_ptr<WatchdogInfo> _watchdog_info;
    rclcpp::TimerBase::SharedPtr _rewait_timer;
    bool _rewaiting = false;

    ActivePhase(
      agv::RobotContextPtr context,
      std::string lift_name,
      std::string destination,
      Data data);

    void _init_obs();

    LegacyTask::StatusMsg _get_status(
      const rmf_lift_msgs::msg::LiftState::SharedPtr& lift_state);

    void _do_publish();
    bool _finish();
  };

  class PendingPhase : public LegacyTask::PendingPhase
  {
  public:

    PendingPhase(
      agv::RobotContextPtr context,
      std::string lift_name,
      std::string destination,
      Data data);

    std::shared_ptr<LegacyTask::ActivePhase> begin() override;

    rmf_traffic::Duration estimate_phase_duration() const override;

    const std::string& description() const override;

    const std::string& lift_name() const
    {
      return _lift_name;
    }

    Data& data()
    {
      return _data;
    }

  private:
    agv::RobotContextPtr _context;
    std::string _lift_name;
    std::string _destination;
    rmf_traffic::Time _expected_finish;
    std::string _description;
    Data _data;
  };
};

} // namespace phases
} // namespace rmf_fleet_adapter

#endif // SRC__RMF_FLEET_ADAPTER__PHASES__REQUESTLIFT_HPP
