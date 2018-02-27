
#include "can.h"


typedef struct{
    uint8_t error_index;
    void (*CanErrorHandler)(void);
}CAN_ERROR_TypeDef;


static void Can_ErrorHandling(void);
static void CanBusOff(void);
static void CanBusOffRecover(void);

CAN_ERROR_TypeDef can_error[] = {
    { RSCAN_BUS_ERROR,Can_ErrorHandling},
    { RSCAN_ERROR_WARNING,Can_ErrorHandling},
    { RSCAN_ERROR_PASSIVE,Can_ErrorHandling},
    { RSCAN_BUS_OFF_ENTRY,CanBusOff},
    { RSCAN_BUS_OFF_RECOVERY,CanBusOffRecover},
    { RSCAN_OVERLOAD_ERROR,Can_ErrorHandling},
    { RSCAN_BUS_DOMINANT_LOCK,Can_ErrorHandling},
    { RSCAN_ARBITRATION_LOST,Can_ErrorHandling},
    { RSCAN_STUFF_ERROR,Can_ErrorHandling},
    { RSCAN_FORM_ERROR,Can_ErrorHandling},
    { RSCAN_ACK_ERROR,Can_ErrorHandling},
    { RSCAN_CRC_ERROR,Can_ErrorHandling},
    { RSCAN_R_BIT_ERROR,Can_ErrorHandling},
    { RSCAN_D_BIT_ERROR,Can_ErrorHandling},
    { RSCAN_ACK_DELIMITER_ERROR,Can_ErrorHandling},
    { RSCAN_DLC_ERROR,Can_ErrorHandling},
};
/* @brief - Initialization of RsCAN module.
 * @param none
 * @reval none
 */
void CanInit(void)
{
    RSCAN_InitTypeDef rscan3;
    RSCAN_RECV_RULE_TypeDef rule[2];
    RSCAN_TrRe_FIFO_CONIFG_PARAM_TypeDef cfg_param[2];

    rscan3.channel = 4;
    rscan3.sp.fcan_src = 1;
    rscan3.sp.bit_time = CANbaudrateSet(CAN_BAUDRATE_250K);

    {//for example
        rule[0].r_pointer.dlc_t = RSCAN_DLC_CHECK_DISABLED;
        rule[0].r_pointer.label_t = 0x891;
        rule[0].r_pointer.recv_buf = RSCAN_RECV_BUF;
        rule[0].r_pointer.recv_buf_index = 0;
        rule[0].r_pointer.k_index = 0;
        rule[0].r_pointer.x_index = 0;
        rule[0].r_id_info.ide = RSCAN_RECV_IDE_STD;
        rule[0].r_id_info.rtr = RSCAN_RECV_DATA_FRM;
        rule[0].r_id_info.target_msg = RSCAN_RECV_FROM_OTHER;
        rule[0].r_id_info.id = 0x123;
        rule[0].r_id_info.mask = CAN_GAFLIDEM_MASK|CAN_GAFLRTRM_MASK |CAN_GAFLIDM_MASK;


        rule[1].r_pointer.dlc_t = RSCAN_DLC_CHECK_DISABLED;
        rule[1].r_pointer.label_t = 0x745;
        rule[1].r_pointer.recv_buf = RSCAN_TrReFIFO;
        rule[1].r_pointer.recv_buf_index = 1;
        rule[1].r_pointer.k_index = 0;
        rule[1].r_pointer.x_index = 0;
        rule[1].r_id_info.ide = RSCAN_RECV_IDE_STD;
        rule[1].r_id_info.rtr = RSCAN_RECV_DATA_FRM;
        rule[1].r_id_info.target_msg = RSCAN_RECV_FROM_OTHER;
        rule[1].r_id_info.id = 0x456;
        rule[1].r_id_info.mask = CAN_GAFLIDEM_MASK|CAN_GAFLRTRM_MASK |CAN_GAFLIDM_MASK;

    }
    {
        cfg_param[0].k_index = 12;
        cfg_param[0].param_un.param_bits.trans_buf_num_linked= 0;
        cfg_param[0].param_un.param_bits.mode= 0x01;
        cfg_param[0].param_un.param_bits.int_req_tm = 0x01;
        cfg_param[0].param_un.param_bits.int_src_sel = 0;
        cfg_param[0].param_un.param_bits.buf_depth = 0x01;


    }
    rscan3.rule_num = ARRAY_SIZE(rule);
    rscan3.rule_p = rule;
    rscan3.trans_buf_mask.buf_mask = RSCAN_TRANSMIT_BUF_0;
    rscan3.trans_buf_mask. fifo_link_mask = RSCAN_TRANSMIT_BUF_1 | RSCAN_TRANSMIT_BUF_2 | RSCAN_TRANSMIT_BUF_3;
    RSCAN_Init(&rscan3);
}

/* @brief - Reset RsCAN module,as same as the initialization.
 * @param none
 * @reval none
 */
void CanReset(void)
{
    CanInit();
}

/* @brief - Select the communication speed .
 * @param baudrate - the baudrate slected to set the speed
 * @reval bit_time - return the RSCAN_BIT_TIMING_TypeDef
 */
RSCAN_BIT_TIMING_TypeDef CANbaudrateSet(CAN_BAUDRATE_Type baudrate)
{
    RSCAN_BIT_TIMING_TypeDef bit_time;
    bit_time.sjw = 0x0;// 1Tq
    bit_time.tseg2 = 0x3;// 4Tq
    bit_time.tseg1 = 0x0a;// 11Tq
    switch(baudrate){
        case CAN_BAUDRATE_100K:
            bit_time.brp = 10-1;
            break;
        case CAN_BAUDRATE_125K:
            bit_time.brp = 8-1;
            break;
        case CAN_BAUDRATE_250K:
            bit_time.brp = 4-1;
            break;
        case CAN_BAUDRATE_500K:
            bit_time.brp = 2-1;
            break;
        default:  // default 250K
            bit_time.brp = 4-1;
            break;
    }

    return bit_time;
}

/* @brief - Transmit data using the transmit buffer with the specified buffer id .
 * @param TxbufferId - the transmit buffer id used to send the data
 * @reval none
 */
void CanTransmitBuffer(uint8_t TxbufferId)
{
    uint8_t data[8] = {0x45,0x56,0x78,0x89,0x90};
    uint8_t date_len = 8;

    CanTransmit(TxbufferId,0x123,date_len,data);
}

/* @brief - Transmit data using the transmit buffer with the specified buffer id ,
            ID, the length of data and the pointer to data array
 * @param TxbufferId - the transmit buffer id used to send the data
 * @param ID -  the ID of the message to be transmitted from the transmit buffer
 * @param Length - the length of the message
 * @param data_p - the pointer to data array contains the message
 * @reval -1 - send failed, positive number - the size of data transmited successfully
 */
int8_t CanTransmit(uint8_t TxbufferId,uint32_t ID,uint8_t Length,uint8_t *data_p)
{
    uint8_t sent_size;
    RSCAN_TRANSMIT_ID_INFO_TypeDef id_info;

    id_info.index = TxbufferId;
    id_info.ide = 0;
    id_info.rtr = 0;
    id_info.id = ID;

    if(__RSCAN_GET_TRANSMIT_STAT(id_info.index,CAN_TMTRM_MASK))
        return -1;

    sent_size = RSCAN_Transmit_Buffer_Write(id_info,Length,data_p);

    return sent_size;
}

/* @brief - Get the state of the data transmission
 * @param TxbufferId - the transmit buffer id used to send the data
 * @reval TRUE - transmit successfully ,otherwise failed.
 */
bool Can_TxConfirmation(uint8_t TxbufferId)
{
    __IO uint8_t ret = 0;

    ret = __RSCAN_GET_TRANSMIT_STAT(TxbufferId,CAN_TMTRF_MASK);

    if(ret == RSCAN_TRANSMIT_COMPLETED_WITHOUT_ABORT ||
        ret == RSCAN_TRANSMIT_COMPLETED_WITH_ABORT){
        ret = 0;
        return TRUE;
    }

    return FALSE;
}

/* @brief - read the message received stored in receive buffer
 * @param RxbufferId - the receive buffer id used to store the message
 * @param p_can_id -  the ID of the message read from receive buffer
 * @param p_dlc - the length of the message received by buffer
 * @param msg_p - the pointer to used to stored the message
 * @reval none
 */
void CanMsgReceived(uint8_t RxbufferId,uint32_t *p_can_id, uint8_t *p_dlc, uint8_t *msg_p)
{
    int8_t ret = -1;
    RSCAN_RECV_ID_INFO_TypeDef id_info;

    id_info.index = RxbufferId;
    while(ret == -1){
        ret  = RSCAN_Receive_Buffer_Read(&id_info,msg_p);
    }

    *p_can_id = id_info.id;
    *p_dlc = ret;
}

/* @brief -  the error-handling functions according to different types of errors.
 * @param global_err - 1: the gobal error , 0 : CAN m error
 * @param channel - the specified channel on which the error occured.
 * @reval none
 */
void Can_ErrorStatus(bool global_err,uint8_t channel)
{
    uint32_t err_ret = 0;
    uint8_t err_index = 0,max_err_index = 0,err_index_offset = 0;

    if(!global_err){
        err_ret = RSCAN_Channel_Error(channel);
        max_err_index = RSCAN_ACK_DELIMITER_ERROR + 1;
    }else{
        err_ret = RSCAN_Global_Error();
        err_index_offset = RSCAN_ACK_DELIMITER_ERROR + 1;
        max_err_index = ARRAY_SIZE(can_error);
    }
    for(err_index = err_index_offset;err_index < max_err_index &&
        (err_ret >> (err_index - err_index_offset));err_index++){
        if(err_index == can_error[err_index].error_index){
            can_error[err_index].CanErrorHandler();
            break;
        }
    }
}

void Can_ErrorHandling(void)
{
    //do nothing,just for test
}

/* @brief - Config the CAN mode, global mode or channel mode
 * @param channel - the specified channel to set the mode
 * @param mode - the specified mode indicated by CAN_MODE_Type
 * @reval none
 */
void CanModeConfig(uint8_t channel,CAN_MODE_Type mode)
{
    if(mode > CAN_CHANNEL_STOP_MODE) return;//mode not supported

    if(mode >= CAN_COMMUNICATION_MODE && mode <= CAN_CHANNEL_STOP_MODE){
        mode -= CAN_COMMUNICATION_MODE;

        RSCAN_Channel_Mode_Ctl(channel,(RSCAN_CHANNEL_MODE_Type)mode,1);
    }else{
        RSCAN_Global_Mode_Ctl((RSCAN_GLOBAL_MODE_Type)mode,1);
    }
}

void CanBusOff(void)
{

}

void CanBusOffRecover(void)
{

}

int8_t CAN_RAM_Test(uint8_t test_page,uint32_t *test_data,uint8_t size)
{
    return  RSCAN_RAM_Test_Perform(test_page,test_data,size);
}