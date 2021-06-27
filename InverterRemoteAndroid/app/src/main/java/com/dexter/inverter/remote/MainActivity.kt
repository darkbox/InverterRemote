package com.dexter.inverter.remote

import android.content.Context
import android.os.Bundle
import android.util.Log
import android.widget.Button
import android.widget.TextView
import android.widget.Toast
import androidx.appcompat.app.AppCompatActivity
import com.androidnetworking.AndroidNetworking
import com.androidnetworking.common.Priority
import com.androidnetworking.error.ANError
import com.androidnetworking.interfaces.JSONObjectRequestListener
import com.androidnetworking.interfaces.StringRequestListener
import org.json.JSONObject
import java.util.*


class MainActivity : AppCompatActivity() {

    private var isBusy = false
    private val service = "http://192.168.1.140:80"
    private lateinit var onBtn: Button
    private lateinit var offBtn: Button
    private lateinit var resetBtn: Button
    private lateinit var statusText: TextView
    private lateinit var mContext: Context

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)
        mContext = this
        AndroidNetworking.initialize(applicationContext)

        onBtn = findViewById(R.id.onButton)
        offBtn = findViewById(R.id.offButton)
        resetBtn = findViewById(R.id.resetButton)
        statusText = findViewById(R.id.statusText)

        onBtn.setOnClickListener {
            doRequest(1, null)
        }
        offBtn.setOnClickListener {
            doRequest(0, null)
        }
        resetBtn.setOnClickListener {
            doRequest(0, object : OnResponseListener {
                override fun onResponse() {
                    isBusy = true
                    val timer = Timer()
                    timer.schedule(object: TimerTask() {
                        override fun run() {
                            runOnUiThread {
                                isBusy = false
                                doRequest(1, null)
                            }
                        }
                    }, 1000)

                }
            })
        }

        statusText.text = getString(R.string.status, getString(R.string.unknown))
        doRequest(null, null)
        statusText.setOnClickListener {
            doRequest(null, null)
        }
    }

    private fun doRequest(state: Int?, onResponseListener: OnResponseListener?)  {
        if (isBusy) {
            return
        }

        isBusy = true
        val request = AndroidNetworking.get(service)
            .setPriority(Priority.HIGH)
            .addHeaders("Connection", "close")
        state?.let {
            request.addQueryParameter("s", it.toString())
        }
        request
            .build()
            .getAsJSONObject(object: JSONObjectRequestListener {
                override fun onResponse(response:JSONObject?) {
                    isBusy = false
                    response?.let {
                        if (it.has("status")) {
                            val status = it.getInt("status")
                            if (status == 1) {
                                // Is ON
                                Toast.makeText(mContext,  getString(R.string.on), Toast.LENGTH_SHORT).show()
                                statusText.text = getString(R.string.status, getString(R.string.on))
                            } else {
                                // Is OFF
                                statusText.text = getString(R.string.status, getString(R.string.off))
                                Toast.makeText(mContext,  getString(R.string.off), Toast.LENGTH_SHORT).show()
                            }
                        }
                    }

                    onResponseListener?.onResponse()
                }

                override fun onError(anError: ANError?) {
                    isBusy = false
                    statusText.text = getString(R.string.status, getString(R.string.unknown))
                    Toast.makeText(mContext, "Error", Toast.LENGTH_SHORT).show()
                    onResponseListener?.onResponse()
                    anError?.let {
                        Log.e("REMOTE", it.message!!)
                    }
                }
            })
    }

    override fun onResume() {
        super.onResume()
        doRequest(null, null)
    }

    interface OnResponseListener {
        fun onResponse()
    }
}