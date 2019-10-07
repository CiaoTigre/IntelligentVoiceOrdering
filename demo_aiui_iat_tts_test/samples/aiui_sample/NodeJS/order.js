

AIUI.create("v2.1",  async function(aiui,  err){
	var Num = Math.ceil(Math.random()*10);
    var requestObject = aiui.getRequest().getObject();
    var response = aiui.getResponse();
    var updatedIntent = aiui.getUpdatedIntent();
    // 判断请求类型
    var requestType =requestObject.request.type;
    console.log("技能请求类型为:" + requestType);
    if(requestType === "LaunchRequest"){
        // 会话保持活动状态
        response.withShouldEndSession(false);
        response.setOutputSpeech("很高兴又见面啦！");
    } else if(requestType === "IntentRequest"){
        // 会话保持活动状态
        response.withShouldEndSession(false);
        // 获取当前意图名
        intentName = requestObject.request.intent.name;
        console.log("本次意图来自:" + intentName);
        switch(intentName){

			case 'StartSkill':

				response.setOutputSpeech("已进入点餐技能，请吩咐！");
				break;
				
            case 'OrderProcess':
                //获取当前对话的填槽状态
                var dialogState= requestObject.request.dialogState;
                //判断填槽状态是否已完成
                if(dialogState != null && dialogState != "COMPLETED") {
					//系统默认按照开发者在平台填写的信息进行追问反问－>托管
					// response.addDelegateDirective();
					var DrinkName = updatedIntent.getSlotValue('DrinkNameSlot');
					var CupNum = updatedIntent.getSlotValue('CupNumSlot');
					var Temp = updatedIntent.getSlotValue('TempSlot');

					if (DrinkName == null){
						response.addElicitSlotDirective("DrinkNameSlot");
       					response.setOutputSpeech(">>>请问你要喝什么饮品呢？");

					}
					if (CupNum == null){
						response.addElicitSlotDirective("CupNumSlot");
       					response.setOutputSpeech(">>>可以告诉我您要几份吗？");
						
					}
					if (Temp == null){
						response.addElicitSlotDirective("TempSlot");
       					response.setOutputSpeech(">>>你喜欢热的还是凉的呢？");
						
					}

                }else{ 
					var DrinkName = updatedIntent.getSlotValue('DrinkNameSlot');
					var CupNum = updatedIntent.getSlotValue('CupNumSlot');
					var Temp = updatedIntent.getSlotValue('TempSlot');

					switch(Num%3){
				
						case 0:
							response.setOutputSpeech('>>>您确定是要' + CupNum + Temp + DrinkName + '这样吗？');
							break;
						case 1:
							response.setOutputSpeech('>>>确认是' + CupNum + Temp + DrinkName + '吗？'); 
							break;
						case 2:
							response.setOutputSpeech( '>>>是' +　CupNum + Temp + DrinkName + '对吗？');
							break;
					}
					
                }
                break;
            
            case 'EndSignal':
                response.setOutputSpeech(">>>好的，立刻为您下单！");  
                break;   

            default:
                response.setOutputSpeech("这是一条来自IntentRequest未知意图的 answer");
                break;
        }
    } else if(requestType === "SessionEndedRequest"){
        response.withShouldEndSession(true);
        response.setOutputSpeech("退出技能！");
    }
    aiui.commit();
})