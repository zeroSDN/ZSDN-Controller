package RESTAdmin;

import java.math.BigInteger;
import java.util.Date;

/**
 * @author Maksim Pahlberg
 * Container for message data
 * used with GSON
 *
 */
public class Message {

	String id;
	String messageType;
	BigInteger senderId;
	BigInteger senderType;
	long timeStamp;
	
	public String getId() {
		return id;
	}
	public void setId(String id) {
		this.id = id;
	}
	public String getMessageType() {
		return messageType;
	}
	public void setMessageType(String messageType) {
		this.messageType = messageType;
	}
	public BigInteger getSenderId() {
		return senderId;
	}
	public void setSenderId(BigInteger senderId) {
		this.senderId = senderId;
	}
	public BigInteger getSenderType() {
		return senderType;
	}
	public void setSenderType(BigInteger senderType) {
		this.senderType = senderType;
	}
	public long getTimeStamp() {
		return timeStamp;
	}
	public void setTimeStamp(long timeStamp) {
		this.timeStamp = timeStamp;
	}
}
