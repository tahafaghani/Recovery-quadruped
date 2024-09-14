/*============================= Stand Up ==============================*/
/**
 * Transitionary state that is called for the robot to stand up into
 * balance control mode.
 */
// Todo: kpCartesian and kdCartesian need to be tuned (57-58). now kp=500, kd=8
// Todo: stand height target need to be decided(53). now = 0.4

#include "FSM_State_StandUp.h"

/**
 * Constructor for the FSM State that passes in state specific info to
 * the generic FSM State constructor.
 *
 * @param _controlFSMData holds all of the relevant control data
 */
template <typename T>
FSM_State_StandUp<T>::FSM_State_StandUp(ControlFSMData<T>* _controlFSMData)
    : FSM_State<T>(_controlFSMData, FSM_StateName::STAND_UP, "STAND_UP"),
_ini_foot_pos(4){
  // Do nothing
  // Set the pre controls safety checks
  this->checkSafeOrientation = false;

  // Post control safety checks
  this->checkPDesFoot = false;
  this->checkForceFeedForward = false;
}

template <typename T>
void FSM_State_StandUp<T>::onEnter() {
  // Default is to not transition
  this->nextStateName = this->stateName;

  // Reset the transition data
  this->transitionData.zero();

  // Reset iteration counter
  iter = 0;

  for(size_t leg(0); leg<4; ++leg){
    _ini_foot_pos[leg] = this->_data->_legController->datas[leg].p;
//    std::cout<<leg<<" "<<_ini_foot_pos[leg]<<std::endl;
  }
}

/**
 * Calls the functions to be executed on each control loop iteration.
 */
template <typename T>
void FSM_State_StandUp<T>::run() {

  if(this->_data->_quadruped->_robotType == RobotType::MINI_CHEETAH) {
    T hMax = 0.25;
    T progress = 2 * iter * this->_data->controlParameters->controller_dt;

    if (progress > 1.){ progress = 1.; }

    for(int i = 0; i < 4; i++) {
      this->_data->_legController->commands[i].kpCartesian = Vec3<T>(500, 500, 500).asDiagonal();
      this->_data->_legController->commands[i].kdCartesian = Vec3<T>(8, 8, 8).asDiagonal();

      this->_data->_legController->commands[i].pDes = _ini_foot_pos[i];
      this->_data->_legController->commands[i].pDes[2] = 
        progress*(-hMax) + (1. - progress) * _ini_foot_pos[i][2];
    }
  }else if (this->_data->_quadruped->_robotType == RobotType::MILAB){
      T hMax = 0.35;
      T progress = 1 * iter * this->_data->controlParameters->controller_dt;

      if (progress > 1.){ progress = 1.; }

      for(int i = 0; i < 4; i++) {
          float kp_cartesian = this->_data->controlParameters->stand_kp_cartesian[0];
          float kd_cartesian = this->_data->controlParameters->stand_kd_cartesian[0];
          this->_data->_legController->commands[i].kpCartesian = Vec3<T>(kp_cartesian,kp_cartesian,kp_cartesian).asDiagonal();
          this->_data->_legController->commands[i].kdCartesian = Vec3<T>(kd_cartesian,kd_cartesian,kd_cartesian).asDiagonal();
          this->_data->_legController->commands[i].forceFeedForward = Vec3<T>(0.f,0.f,-50.f);
          this->_data->_legController->commands[i].pDes = _ini_foot_pos[i];
          this->_data->_legController->commands[i].pDes[2] =
                  progress*(-hMax) + (1. - progress) * _ini_foot_pos[i][2];
      }
  }else if (this->_data->_quadruped->_robotType == RobotType::CHEETAH_3){
      T hMax = 0.45;
      T progress = 2 * iter * this->_data->controlParameters->controller_dt;

      if (progress > 1.){ progress = 1.; }

      for(int i = 0; i < 4; i++) {
          this->_data->_legController->commands[i].kpCartesian = Vec3<T>(500, 500, 500).asDiagonal();
          this->_data->_legController->commands[i].kdCartesian = Vec3<T>(8, 8, 8).asDiagonal();

          this->_data->_legController->commands[i].pDes = _ini_foot_pos[i];
          this->_data->_legController->commands[i].pDes[2] =
                  progress*(-hMax) + (1. - progress) * _ini_foot_pos[i][2];
      }
  }
}

/**
 * Manages which states can be transitioned into either by the user
 * commands or state event triggers.
 *
 * @return the enumerated FSM state name to transition into
 */
template <typename T>
FSM_StateName FSM_State_StandUp<T>::checkTransition() {
  this->nextStateName = this->stateName;
  iter++;

  // Switch FSM control mode 0,1,2,3,6
  switch ((int)this->_data->controlParameters->control_mode) {
    case K_STAND_UP:
      break;

    case K_BALANCE_STAND:
      this->nextStateName = FSM_StateName::BALANCE_STAND;
      break;


    case K_RECOVERY_STAND:
      this->nextStateName = FSM_StateName::RECOVERY_STAND;
      break;

    case K_SQUAT_DOWN:
      this->nextStateName = FSM_StateName::SQUAT_DOWN;
      break;

    case K_PASSIVE:
      this->nextStateName = FSM_StateName::PASSIVE;
      break;

    default:
      std::cout << "[CONTROL FSM] Bad Request: Cannot transition from "
                << K_PASSIVE << " to "
                << this->_data->controlParameters->control_mode << std::endl;
  }

  // Get the next state
  return this->nextStateName;
}

/**
 * Handles the actual transition for the robot between states.
 * Returns true when the transition is completed.
 *
 * @return true if transition is complete
 */
template <typename T>
TransitionData<T> FSM_State_StandUp<T>::transition() {
  // Finish Transition
  switch (this->nextStateName) {
    case FSM_StateName::PASSIVE:  // normal
      this->transitionData.done = true;
      break;

    case FSM_StateName::BALANCE_STAND:
      this->transitionData.done = true;
      break;

    case FSM_StateName::SQUAT_DOWN:
      this->transitionData.done = true;
      break;

    case FSM_StateName::RECOVERY_STAND:
      this->transitionData.done = true;
      break;

    default:
      std::cout << "[CONTROL FSM] Something went wrong in transition"
                << std::endl;
  }

  // Return the transition data to the FSM
  return this->transitionData;
}

/**
 * Cleans up the state information on exiting the state.
 */
template <typename T>
void FSM_State_StandUp<T>::onExit() {
  // Nothing to clean up when exiting
}

// template class FSM_State_StandUp<double>;
template class FSM_State_StandUp<float>;
